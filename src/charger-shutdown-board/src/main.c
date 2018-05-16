#include "main.h"

volatile uint32_t msTicks;
const uint32_t OscRateIn =24000000;

uint8_t can_data[8]; //64 bit can message to be sent to charger

void SysTick_Handler(void){
	msTicks++;
}

int main(void) {

	#define SCL 0, 4
  	#define SDA 0, 5
  	#define SLAVEADDR 0x64

	static I2C_XFER_T xfer;
	static uint8_t i2c_rx_buf[20];
	static uint8_t i2c_tx_buf[20];


//	assuming this is byte 0 need to double check
  	can_data[0]=0xFF; //Charger enable byte
  	can_data[1]=0xE8;
  	can_data[2]=0x3; //Power Reference of 100%
  	can_data[3]= 0x88;
  	can_data[4]=0x8; //Corresponds to DC voltage limit of 300 Volts
  	can_data[5]=0x64; //DC Current limit of 10 A (Charger max)
  	can_data[6]=0x0;
  	can_data[7]=0; //Reserved for charger


  	SystemCoreClockUpdate();
  	Board_GPIO_Init();
	Board_UART_Init(57600);
  	Board_Print("H e l l o\n");
  	CAN_INIT();
  	Board_Print("CAN Initialized\n");
  	//I2C Initialization
	Chip_SYSCTL_PeriphReset(RESET_I2C0); // Reset the I2C Peripheral
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1); // SCL
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1); // SDA

	// LPC_SYSCTL->SYSAHBCLKCTRL |= 0x20; 	// Enable clock and power to I2C block
	Chip_SYSCTL_DeassertPeriphReset(RESET_I2C0);
	Chip_I2C_Init(I2C0);
	Chip_I2C_SetClockRate(I2C0, 100000);

	Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandler);
	NVIC_EnableIRQ(I2C0_IRQn);

	xfer.txBuff = i2c_tx_buf;
	xfer.rxBuff = i2c_rx_buf;
	xfer.slaveAddr = 0x64;


	Board_Print("I2C Initialized\n");

  while(1){
	  advance_csb_state();
  }
  
  return 0;
}



