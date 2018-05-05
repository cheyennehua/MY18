#include "state_csb_root.h"

void enter_csb_state_root(void){
	Board_Print("Entered Root!\n");
}

void update_csb_state_root(void){
	if(Board_Pin_Read(PIN_BMS_FAULT)&&Board_Pin_Read(PIN_IMD_IN)&&Board_Pin_Read(PIN_INTERLOCK))
		set_csb_state(CSB_STATE_PRECHARGE);
}
