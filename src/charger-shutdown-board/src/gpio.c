#include "gpio.h"

bool any_gate_fault() {
  return !Board_Pin_Read(PIN_BMS_FAULT) ||
         !Board_Pin_Read(PIN_IMD_IN)    ||
         !Board_Pin_Read(PIN_INTERLOCK);
}

void Board_GPIO_Init(void) {
  // Digital GPIO Initialization
  Chip_GPIO_Init(LPC_GPIO);

  // LED1
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_LED1);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_LED1, IOCON_LED1_FUNC);
  Chip_GPIO_SetPinState(LPC_GPIO, PIN_LED1, false);

  // LED2
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_LED2);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_LED2, IOCON_LED2_FUNC);
  Chip_GPIO_SetPinState(LPC_GPIO, PIN_LED2, false);

  // Charge Enable Pin
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, PIN_CHARGER_ENABLE);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_CHARGER_ENABLE,
                       (IOCON_FUNC0 | IOCON_DIGMODE_EN | IOCON_MODE_INACT));

  // BMS Fault Pin
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, PIN_BMS_FAULT);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_BMS_FAULT,
                       (IOCON_FUNC0 | IOCON_DIGMODE_EN | IOCON_MODE_INACT));

  // IMD Fault Pin
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, PIN_IMD_IN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_IMD_IN,
                       (IOCON_FUNC0 | IOCON_DIGMODE_EN | IOCON_MODE_INACT));

  // Interlock Fault Pin
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, PIN_INTERLOCK);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_INTERLOCK,
                       (IOCON_FUNC0 | IOCON_DIGMODE_EN | IOCON_MODE_INACT));

  // Precharge Pin
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_PRECHARGE);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_PRECHARGE, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, PIN_PRECHARGE, false);

  // Contactors Closed
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, PIN_HIGH_SIDE_CONTACTOR);
  Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_IOCON_CONTACTORS_CLOSED,
                       (IOCON_FUNC0 | IOCON_DIGMODE_EN | IOCON_MODE_INACT));

  // I2C Initialization

  Chip_SYSCTL_PeriphReset(RESET_I2C0);                        // Reset the I2C
                                                              // Peripheral
  Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1); // SCL
  Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1); // SDA
}

inline void Board_Pin_Set(uint8_t led_gpio, uint8_t led_pin, bool state) {
  // Set the value of a GPIO pin
  Chip_GPIO_SetPinState(LPC_GPIO, led_gpio, led_pin, state);
}

inline bool Board_Pin_Read(uint8_t port, uint8_t pin) {
  // Read the value of a GPIO pin
  return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}

inline void Board_Pin_Toggle(uint8_t port, uint8_t pin) {
  // Toggle a GPIO pin
  Chip_GPIO_SetPinState(LPC_GPIO, port, pin, !Board_Pin_Read(port, pin));
}