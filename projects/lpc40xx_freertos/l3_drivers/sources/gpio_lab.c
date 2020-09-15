#include "gpio_lab.h"
#include "lpc40xx.h"

void gpiolab__set_as_input(uint8_t pin_num) { LPC_GPIO1->DIR &= ~(1U << pin_num); }

/// Should alter the hardware registers to set the pin as output
void gpiolab__set_as_output(uint8_t pin_num) { LPC_GPIO1->DIR |= (1U << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpiolab__set_high(uint8_t pin_num) {
  gpiolab__set_as_output(pin_num);
  LPC_GPIO1->SET = (1U << pin_num);
}

/// Should alter the hardware registers to set the pin as low
void gpiolab__set_low(uint8_t pin_num) {
  gpiolab__set_as_output(pin_num);
  LPC_GPIO1->CLR = (1U << pin_num);
}

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpiolab__set(uint8_t pin_num, bool high) {
  if (high == true) {
    LPC_GPIO1->SET = (1U << pin_num);
  } else {
    LPC_GPIO1->CLR = (1U << pin_num);
  }
}

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpiolab__get_level(uint8_t pin_num) {
  bool level = false;
  level = (bool)(LPC_GPIO1->PIN & (1U << pin_num));

  return level;
}