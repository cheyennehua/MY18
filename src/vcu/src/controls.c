#include "controls.h"
#include "eff.h"

static bool enabled = false;
static int32_t torque_command = 0;
static int32_t speed_command = 0;

can0_VCUControlsParams_T control_settings = {};
can0_PowerLimMonitoring_T power_lim_monitoring = {};
can0_VCU_PowerLimSettings_T power_lim_settings = {};

static int32_t hinge_limiter(int32_t x, int32_t m, int32_t e, int32_t c);

static int16_t get_eff_percent(int32_t torque, int32_t speed);

const int8_t num_pole_pairs = 10;
const int16_t flux = 355; // Vs * 10 ** -4

static int32_t get_power_limited_torque_vq(void) {
  if (mc_readings.V_VBC_Vq >= 0) return MAX_TORQUE;

  uint8_t rms_eff_percent = get_eff_percent(mc_readings.last_commanded_trq / 10, mc_readings.speed);

  power_lim_monitoring.calc_eff = rms_eff_percent;
  printf("Eff percent: %d\r\n", rms_eff_percent);

  // Motor constant times 10e4
  int32_t motor_constant_10e4 = 3 * num_pole_pairs * flux / 2;

  // Convert to W from W / 100
  int32_t plim_W = power_lim_settings.power_lim * 100;

  // Powers of 10:
  // +1 Multiply by 10 to divide numerater by 10, which gives dV to V
  // -2 Divide by 100 to covert from percentage points to fraction
  // ------------------------------------------------------------------
  // - 1 total
  int32_t allowed_iq = -1 * rms_eff_percent * plim_W / (mc_readings.V_VBC_Vq * 10);

  printf("Plim W: %ld\t|Vq| in dV: %d\tAllowed Iq: %ld\r\n", plim_W, -mc_readings.V_VBC_Vq, allowed_iq);

  int32_t undivided_iq = (allowed_iq) * motor_constant_10e4;

  printf("Undivided Iq: %d\r\n", undivided_iq);

  // Powers of 10:
  // +1 convert from Nm to dNm
  // -4 Divide out 10e4 in motor_constant_10e4
  int32_t calc_torq = undivided_iq / 1000;

  if (calc_torq < MAX_TORQUE) {
    return calc_torq;
  }
  return MAX_TORQUE;
}

static int32_t get_power_limited_torque_mech(void) {
  if (mc_readings.speed >= 0) return MAX_TORQUE;

  int32_t plim_W = power_lim_settings.power_lim * 100;

  printf("plim_W: %d\r\n", plim_W);

  // Convert RPM to rad/s with 2pi/60, *10 to dNm, *100 for dkW to W
  return 10 * plim_W / (-mc_readings.speed * 628 / 6000);
}

int32_t get_power_limited_torque(int32_t pedal_torque) {
  int32_t tMAX_vq = get_power_limited_torque_vq();
  int32_t tMAX_mech = get_power_limited_torque_mech();

  int32_t pwr_lim_trq;
  if (power_lim_settings.using_vq_lim) {
    pwr_lim_trq = tMAX_vq;
    printf("vq tmax = %d\r\n", tMAX_vq);
  } else {
    pwr_lim_trq = tMAX_mech;
  }

  power_lim_monitoring.vq_tmax = tMAX_vq;
  power_lim_monitoring.mech_tmax = tMAX_mech;

  if (pwr_lim_trq > MAX_TORQUE) pwr_lim_trq = MAX_TORQUE;

  printf("FINAL PWR LIMITED TRQ: %d\r\n", pwr_lim_trq);
  return pwr_lim_trq;
}

// PRIVATE FUNCTIONS
static int32_t get_torque(void);
static int32_t get_regen_torque(void);
static int32_t get_temp_limited_torque(int32_t pedal_torque);
static int32_t get_voltage_limited_torque(int32_t pedal_torque);

void init_controls_defaults(void) {
  control_settings.using_regen = false;
  control_settings.regen_bias = 57;
  control_settings.limp_factor = 100;
  control_settings.temp_lim_min_gain = 25;
  control_settings.temp_lim_thresh_temp = 50;
  control_settings.volt_lim_min_gain = 0;
  control_settings.volt_lim_min_voltage = 300;
  control_settings.torque_temp_limited = false;

  power_lim_settings.power_lim = 800;
  power_lim_settings.using_pl = false;
}

void enable_controls(void) {
  enabled = true;
  torque_command = 0;
  speed_command = 0;

  unlock_brake_valve();
}

void disable_controls(void) {
  enabled = false;
  torque_command = 0;
  speed_command = 0;

  set_brake_valve(false);
  lock_brake_valve();
}

bool get_controls_enabled(void) {
  return enabled;
}

void execute_controls(void) {
  if (!enabled) return;

  torque_command = get_torque();
  controls_monitoring.raw_torque = torque_command;

  // Control regen brake valve:
  bool brake_valve_state = control_settings.using_regen && get_pascals(pedalbox.REAR_BRAKE) < RG_REAR_BRAKE_THRESH;
  set_brake_valve(brake_valve_state);

  int32_t regen_torque;
  if (brake_valve_state) {
    regen_torque = get_regen_torque();
  } else {
    regen_torque = 0;
  }

  // Calculate commands to set gain values
  (void) get_voltage_limited_torque(torque_command);
  (void) get_temp_limited_torque(torque_command);

  // Extra check to ensure we are only sending regen torque when allowed
  if (torque_command == 0) {
    torque_command = regen_torque;
  }
  else {
    // Only use limits when we're not doing regen
    int32_t power_limited_torque = get_power_limited_torque(torque_command);

    int32_t voltage_limited_torque = get_voltage_limited_torque(torque_command);

    if (!control_settings.using_voltage_limiting) voltage_limited_torque = torque_command;

    int32_t temp_limited_torque = get_temp_limited_torque(torque_command);

    if (!control_settings.using_temp_limiting) temp_limited_torque = torque_command;

    control_settings.torque_temp_limited = temp_limited_torque < torque_command;

    int32_t min_sensor_torque;
    if (voltage_limited_torque < temp_limited_torque) {
      min_sensor_torque = voltage_limited_torque;
    }
    else {
      min_sensor_torque = temp_limited_torque;
    }

    if (power_lim_settings.using_pl && power_limited_torque < min_sensor_torque) {
        min_sensor_torque = power_limited_torque;
    }

    int32_t dash_limited_torque = torque_command * control_settings.limp_factor / 100;

    int32_t limited_torque;
    if (dash_limited_torque < min_sensor_torque) {
      limited_torque = dash_limited_torque;
    }
    else {
      limited_torque = min_sensor_torque;
    }

    torque_command = limited_torque;
  }

  sendTorqueCmdMsg(torque_command);
}

static int32_t get_torque(void) {
  int32_t accel = pedalbox_avg(accel);

  if (accel < PEDALBOX_ACCEL_RELEASE) return 0;

  return MAX_TORQUE * (accel - PEDALBOX_ACCEL_RELEASE) / (MAX_ACCEL_VAL - PEDALBOX_ACCEL_RELEASE);
}

static int32_t get_regen_torque() {
  int32_t regen_torque = 0;

  // IMU velocity component unit conversion:
  // Starting units: 100 * m/s, ending units: kph
  // (100 m/s) * (1 km/1000 m) * (3600 s/1 hr) = 360 kph
  // So multiply by 360 to get kph --> but this is squared, so multiply by 129600
  int32_t imu_vel_north_squared = imu_velocity.north * imu_velocity.north;
  int32_t imu_vel_east_squared = imu_velocity.east * imu_velocity.east;
  int32_t car_speed_squared = 129600 * (imu_vel_north_squared + imu_vel_east_squared);

  if ((mc_readings.speed * -1 > RG_MOTOR_SPEED_THRESH) &&
      (cs_readings.V_bus < RG_BATTERY_VOLTAGE_MAX_THRESH) &&
      (get_pascals(pedalbox.FRONT_BRAKE) > RG_FRONT_BRAKE_THRESH)) {
      // car_speed_squared > RG_CAR_SPEED_THRESH * RG_CAR_SPEED_THRESH) { // Threshold is 5 kph, so 25 (kph)^2
    int32_t kilo_pascals = get_pascals(pedalbox.FRONT_BRAKE) / 1000;
    // Because we already divded by 1000 by using kilo_pascals instead of
    // pascals, we only need to divde by 10^4, not 10^7 for RG_10_7_K
    // And by dividing by 10^3 instead of 10^4, we are multiplying by 10 to get
    // dNm instead of Nm
    regen_torque = RG_10_7_K * kilo_pascals * (100 - control_settings.regen_bias) /(control_settings.regen_bias * 1000);
    if (regen_torque > RG_TORQUE_COMMAND_MAX) {
      regen_torque = RG_TORQUE_COMMAND_MAX;
    }
  }

  // Regen is negative torque, and we've calculated a positive number so far
  return -1 * regen_torque;
}

static int32_t get_temp_limited_torque(int32_t pedal_torque) {
  uint32_t temp_sum = 0;
  for (uint32_t i = 0; i < TEMP_LOG_LENGTH; i++) {
    temp_sum += cell_readings.temp_log[i];
  }

  // For higher accuracy (to centi-Celsius) multiply by 10 before dividing
  uint32_t temp_cC = temp_sum * 10 / TEMP_LOG_LENGTH;
  controls_monitoring.filtered_temp = temp_cC;

  // Thresh was in degrees, so multiply it by 100
  int32_t thresh_cC = control_settings.temp_lim_thresh_temp * 100;

  // static uint32_t lastt = 0;
  // if (HAL_GetTick() - lastt > 100) {
  //   printf("Filtered Temp: %d\tTemp threshold: %d\r\n", temp_cC, thresh_cC);
  //
  //   lastt = HAL_GetTick();
  // }

  int32_t gain = hinge_limiter(temp_cC, control_settings.temp_lim_min_gain, thresh_cC, MAX_TEMP);
  controls_monitoring.tl_gain = gain;

  return gain * pedal_torque / 100;
}

static int32_t get_voltage_limited_torque(int32_t pedal_torque) {
  // We want cs_readings.V_bus/72 - 0.1 because of empirical differences
  // We also want centivolts instead of milivolts, so this gives us:
  // (cs_readings.V_bus/72)/10 - 1/10 = (cs_readings.V_bus - 72)/720
  int32_t cell_voltage = (cs_readings.V_bus - 72) / 720;
  controls_monitoring.voltage_used = cell_voltage;

  // static uint32_t lastt = 0;
  // if (HAL_GetTick() - lastt > 100) {
  //   printf("Voltage: %d\tVoltage threshold: %d\r\n", cell_voltage, control_settings.volt_lim_min_voltage);
  //
  //   lastt = HAL_GetTick();
  // }

  int32_t gain = hinge_limiter(cell_voltage, control_settings.volt_lim_min_gain, control_settings.volt_lim_min_voltage, MIN_VOLTAGE);

  controls_monitoring.vl_gain = gain;
  return gain * pedal_torque / 100;
}

// Returns the output of a linear hinge.
// It is a continuous function.
// `m` is the minimum output of this function.
// `c` is the `x` threshold above which the function returns `m`.
// for `m` < `x` < `c` the output is linearly decreasing.
int32_t positive_hinge(int32_t x, int32_t m, int32_t c) {
  if (x < 0) return 100;
  if (x > c) return m;

  return (x * (m - 100) / c) + 100;
}

// Returns the output of a bidirectional linear hinge limiter.
int32_t hinge_limiter(int32_t x, int32_t m, int32_t e, int32_t c) {
  if (c > e) return positive_hinge(x - e, m, c - e);
  else       return positive_hinge(e - x, m, e - c);
}

int16_t get_eff_percent(int32_t torque, int32_t speed) {
  int16_t torq_idx = torque * NUM_TRQ_INDXS / 240;
  int16_t spd_idx = speed * NUM_SPD_INDXS / 6000;
  return data_eff_percent[spd_idx][torq_idx];
}
