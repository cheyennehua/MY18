#include "dispatch.h"

#include "board.h"
#include "button_listener.h"
#include "page_manager.h"
#include "carstats.h"
#include "NHD_US2066.h"
#include "led.h"

static button_state_t left_button;
static button_state_t right_button;

static page_manager_t page_manager;
static carstats_t carstats;

static NHD_US2066_OLED oled;

static uint32_t nextOLEDUpdate;

static bool active_aero_enabled = false;

#define BUTTON_DOWN false
#define OLED_UPDATE_INTERVAL_MS 50

// dead after (ms)
#define BMS_HEARTBEAT_EXPIRE 1000

void update_lights(void);
void send_dash_request(can0_DashRequest_type_T type);

void dispatch_init() {
    init_button_state(&left_button);
    init_button_state(&right_button);

    page_manager_init(&page_manager, &carstats);

    oled_init(&oled);
    Delay(100);
    oled_init_commands(&oled);
    Delay(100);
    oled_clear(&oled);
    Delay(100);

    nextOLEDUpdate = 0;

    // init carstats fields
    carstats.battery_voltage         = -1;
    carstats.lowest_cell_voltage     = -1;
    carstats.max_cell_temp           = -1;
    carstats.power                   = -1;
    carstats.soc                     = -1;
    carstats.torque_mc               = -1;
    carstats.motor_rpm               = -1;
    carstats.front_left_wheel_speed  = -1;
    carstats.front_right_wheel_speed = -1;
    carstats.rear_left_wheel_speed   = -1;
    carstats.rear_right_wheel_speed  = -1;
}

void dispatch_update() {
    /*
    Frame frame;
    handle_can_error(Can_RawRead(&frame));

    can0_T msgType = identify_can0(&frame);
    oled_clearline(&oled, 0);
    oled_set_pos(&oled, 0, 0);
    if (msgType != CAN_UNKNOWN_MSG) {
        oled_print_num(&oled, frame.id);
    } else {
        oled_print(&oled, "Unknown");
    }

    oled_update(&oled);
    */


    bool left_button_down  = (Pin_Read(PIN_BUTTON1) == BUTTON_DOWN);
    bool right_button_down = (Pin_Read(PIN_BUTTON2) == BUTTON_DOWN);
    update_button_state(&left_button, left_button_down);
    update_button_state(&right_button, right_button_down);

    if (right_button.action == BUTTON_ACTION_TAP) {
        page_manager_next_page(&page_manager);

        active_aero_enabled = !active_aero_enabled;
        if (active_aero_enabled) {
            send_dash_request(can0_DashRequest_type_ACTIVE_AERO_ENABLE);
        } else {
            send_dash_request(can0_DashRequest_type_ACTIVE_AERO_DISABLE);
        }
    } else if (right_button.action == BUTTON_ACTION_HOLD) {
        page_manager_prev_page(&page_manager);
    }

    can_update_carstats(&carstats);
    update_lights();

    if (msTicks > nextOLEDUpdate) {
        nextOLEDUpdate = msTicks + OLED_UPDATE_INTERVAL_MS;
        page_manager_update(&page_manager, &oled);
        oled_update(&oled);

    }
}

void update_lights(void) {
    if (carstats.vcu_data.rtd_light_on)
        LED_RTD_on();
    else
        LED_RTD_off();

    if (carstats.vcu_data.hv_light_on)
        LED_HV_on();
    else
        LED_HV_off();

    if (carstats.vcu_data.imd_light_on)
        LED_IMD_on();
    else
        LED_IMD_off();

    if (msTicks < carstats.last_bms_heartbeat + BMS_HEARTBEAT_EXPIRE)
        LED_AMS_on();
    else
        LED_AMS_off();

}

void send_dash_request(can0_DashRequest_type_T type) {
    can0_DashRequest_T msg;
    msg.type = type;
    
    handle_can_error(can0_DashRequest_Write(&msg));
}
