
#include "lv_only.h"

void initLVOnly() {
	printf("\r\nCAR STARTED IN LV MODE\r\n");
}

void loopLVOnly() {
	if (button_presses.DriverReset) {
		changeCarMode(CAR_STATE_PRECHARGING);
	}
}
