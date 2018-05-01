#ifndef _INPUT_H_
#define _INPUT_H_

#include "types.h"
#include "chip.h"

#include "adc.h"

#define ACCEL_SCALE_MAX 1000

#define ADC_UPDATE_PERIOD_MS 1

#define ACCEL_1_LOWER_BOUND 138
#define ACCEL_1_UPPER_BOUND 488

#define ACCEL_2_LOWER_BOUND 256
#define ACCEL_2_UPPER_BOUND 881

// Some wheel speed stuff (copied from MY17)
#define WHEEL_SPEED_TIMEOUT_MS 100
#define WHEEL_SPEED_READ_PERIOD_MS 10

// Microsecond = 1 millionth of a second
#define MICROSECONDS_PER_SECOND_F 1000000.0
// 1000 millirevs = 1 rev
#define MILLIREVS_PER_REV_F 1000.0
#define SECONDS_PER_MINUTE 60

void Input_initialize(void);
void Input_fill_input(void);
void Input_handle_interrupt(uint32_t msTicks, uint32_t curr_tick, Wheel_T wheel);


#endif // _INPUT_H_
