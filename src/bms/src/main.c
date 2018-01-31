#include <string.h>
#include "eeprom_config.h"
#include "chip.h"
#include "ltc6804.h"
#include "board.h"
#include "pack.h"
#include "state_types.h"
#include "config.h"
#include "console.h"
#include "ssm.h"
#include "fsae_can.h"
#include "error_handler.h"

#define EEPROM_CS_PIN 0, 7


extern volatile uint32_t msTicks;

static char str[10];
static char testString[10];

static BMS_PACK_STATUS_T pack_status;
static BMS_INPUT_T bms_input;
static BMS_OUTPUT_T bms_output;
static BMS_STATE_T bms_state;

static BMS_PACK_CONFIG_T pack_config;
static uint32_t cell_voltages[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
static int16_t cell_temperatures[MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE];
static uint8_t module_cell_count[MAX_NUM_MODULES];

// memory for console
static microrl_t rl;
static CONSOLE_OUTPUT_T console_output;

void Init_BMS_Structs(void){

    bms_state.pack_config = &pack_config;

    //Default pack configuration, from pack.h
    pack_config.module_cell_count = module_cell_count;
    pack_config.cell_min_mV = CELL_MIN_mV;
    pack_config.cell_max_mV = CELL_MAX_mV;
    pack_config.cell_capacity_cAh = CELL_CAPACITY_cAh;
    pack_config.num_modules = NUM_MODULES;
    pack_config.cell_charge_c_rating_cC = CELL_CHARGE_C_RATING_cC;
    pack_config.bal_on_thresh_mV = BALANCE_ON_THRESHOLD_mV;
    pack_config.bal_off_thresh_mV = BALANCE_OFF_THRESHOLD_mV;
    pack_config.pack_cells_p = PACK_CELLS_PARALLEL;
    pack_config.cv_min_current_mA = CV_MIN_CURRENT_mA;
    pack_config.cv_min_current_ms = CV_MIN_CURRENT_ms;
    pack_config.cc_cell_voltage_mV = CC_CELL_VOLTAGE_mV;
    pack_config.cell_discharge_c_rating_cC = CELL_DISCHARGE_C_RATING_cC; // at 27 degrees C
    pack_config.max_cell_temp_dC = MAX_CELL_TEMP_dC;
    pack_config.fan_on_threshold_dC = FAN_ON_THRESHOLD_dC;
    pack_config.min_cell_temp_dC = MIN_CELL_TEMP_dC;

    uint8_t i;
    for(i = 0; i < MAX_NUM_MODULES; i++) {
        pack_config.module_cell_count[i] = MODULE_CELL_COUNT;
    }

    bms_input.pack_status = &pack_status;
    bms_input.mode_request = BMS_SSM_MODE_STANDBY;
    bms_input.contactors_closed = false;
    bms_input.msTicks = msTicks;
    bms_input.vcu_mode_request = BMS_SSM_MODE_STANDBY;
    bms_input.csb_mode_request = BMS_SSM_MODE_STANDBY;
    bms_input.ltc_packconfig_check_done = false;
    bms_input.eeprom_read_error = false;

    pack_status.cell_voltages_mV = cell_voltages;
    pack_status.cell_temperatures_dC = cell_temperatures;
    pack_status.pack_cell_max_mV = 0;
    pack_status.pack_cell_min_mV = 0xFFFFFFFF;
    pack_status.pack_current_mA = 0;
    pack_status.pack_voltage_mV = 0;
    pack_status.max_cell_temp_dC = 0;

    pack_status.min_cell_temp_dC = -100;
    pack_status.avg_cell_temp_dC = 0;
    pack_status.min_cell_temp_position = 0;
    pack_status.max_cell_temp_position = 0;

    bms_state.curr_mode = BMS_SSM_MODE_INIT;
}

void Process_Input(BMS_INPUT_T* bms_input) {
    if(bms_state.curr_mode != BMS_SSM_MODE_INIT) {
        Can_Receive(bms_input);
        Board_GetModeRequest(&console_output, bms_input);
        Board_LTC6804_ProcessInputs(&pack_status,&bms_state);
    }

    bms_input->msTicks = msTicks;
    bms_input->contactors_closed = Board_Contactors_Closed();
    bms_input->contactor_weld_one = Board_Contactor_Weld_One();
    bms_input->contactor_weld_two = Board_Contactor_Weld_Two();
}

void Process_Output(BMS_INPUT_T* bms_input,BMS_OUTPUT_T* bms_output, BMS_STATE_T* bms_state) {
    Board_Contactors_Set(bms_output->close_contactors);
    if(bms_output->ltc_deinit){
        Board_LTC6804_DeInit();
    } else if(bms_output->check_packconfig_with_ltc) {
        bms_input->ltc_packconfig_check_done = Board_LTC6804_Init(&pack_config, cell_voltages);
    } else {
        Board_LTC6804_ProcessOutput(bms_output->balance_req);
        Can_Transmit(bms_input, bms_output);
    }


}

void Process_Keyboard(void) {
    uint32_t readln = Board_Read(str,50);
    uint32_t i;
    for(i = 0; i < readln; i++) {
        microrl_insert_char(&rl, str[i]);
    }
}

int main(void) {

    Init_BMS_Structs();
	Board_Chip_Init();
	Board_GPIO_Init();
    Board_UART_Init(57600);
	Board_CAN_Init(CAN_BAUD);
    EEPROM_Init(LPC_SSP1, EEPROM_BAUD, EEPROM_CS_PIN);
    SSM_Init(&bms_input,&bms_state, &bms_output);
    EEPROM_WriteCCPage_Num(0,11);

    //setup readline/console
    microrl_init(&rl, Board_Print);
    microrl_set_execute_callback(&rl,executerl);
    console_init(&bms_input, &bms_state, &console_output);

    while(1) {
        Process_Keyboard(); //console input
        Process_Input(&bms_input); //Processes Inputs(can messages, pin states, cell stats)
        SSM_Step(&bms_input, &bms_state, &bms_output); //changes state based on inputs
        Process_Output(&bms_input,&bms_output,&bms_state); //Transmits can messages, processes ltc output(update balance states)



        if (Error_Handle(bms_input.msTicks) == HANDLER_HALT) {
            break; // Handler requested a Halt
        }
    }
    Board_Println("FORCED HANG");
    Write_EEPROM_Error();

    bms_output.close_contactors = false;

    while(1) {
        bms_input.msTicks = msTicks;
        Process_Output(&bms_input, &bms_output, &bms_state);
        Process_Keyboard();
        bms_state.curr_mode = BMS_SSM_MODE_STANDBY;
    }

	return 0;
}
