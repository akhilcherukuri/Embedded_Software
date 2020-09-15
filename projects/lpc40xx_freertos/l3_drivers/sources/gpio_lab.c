#include "gpio_lab.h"
#include "lpc40xx.h"

void gpiolab__set_as_input(uint8_t pin_num, uint8_t port_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->DIR &= ~(1U << pin_num);
    break;
  case 1:
    LPC_GPIO1->DIR &= ~(1U << pin_num);
    break;
  case 2:
    LPC_GPIO2->DIR &= ~(1U << pin_num);
    break;
  default:
    break;
  }
}

/// Should alter the hardware registers to set the pin as output
void gpiolab__set_as_output(uint8_t pin_num, uint8_t port_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->DIR |= (1U << pin_num);
    break;
  case 1:
    LPC_GPIO1->DIR |= (1U << pin_num);
    break;
  case 2:
    LPC_GPIO2->DIR |= (1U << pin_num);
    break;
  default:
    break;
  }
}

/// Should alter the hardware registers to set the pin as high
void gpiolab__set_high(uint8_t pin_num, uint8_t port_num) {
  gpiolab__set_as_output(pin_num, port_num);
  switch (port_num) {
  case 0:
    LPC_GPIO0->SET = (1U << pin_num);
    break;
  case 1:
    LPC_GPIO1->SET = (1U << pin_num);
    break;
  case 2:
    LPC_GPIO2->SET = (1U << pin_num);
    break;
  default:
    break;
  }
}

/// Should alter the hardware registers to set the pin as low
void gpiolab__set_low(uint8_t pin_num, uint8_t port_num) {
  gpiolab__set_as_output(pin_num, port_num);
  switch (port_num) {
  case 0:
    LPC_GPIO0->CLR = (1U << pin_num);
    break;
  case 1:
    LPC_GPIO1->CLR = (1U << pin_num);
    break;
  case 2:
    LPC_GPIO2->CLR = (1U << pin_num);
    break;
  default:
    break;
  }
}

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpiolab__set(uint8_t pin_num, uint8_t port_num, bool high) {
  switch (port_num) {
  case 0:
    if (high == true) {
      LPC_GPIO0->SET = (1U << pin_num);
    } else {
      LPC_GPIO0->CLR = (1U << pin_num);
    }
    break;
  case 1:
    if (high == true) {
      LPC_GPIO1->SET = (1U << pin_num);
    } else {
      LPC_GPIO1->CLR = (1U << pin_num);
    }
    break;
  case 2:
    if (high == true) {
      LPC_GPIO2->SET = (1U << pin_num);
    } else {
      LPC_GPIO2->CLR = (1U << pin_num);
    }
    break;
  default:
    break;
  }
}

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpiolab__get_level(uint8_t pin_num, uint8_t port_num) {
  bool level = false;
  switch (port_num) {
  case 0:
    level = (bool)(LPC_GPIO0->PIN & (1U << pin_num));
    break;
  case 1:
    level = (bool)(LPC_GPIO1->PIN & (1U << pin_num));
    break;
  case 2:
    level = (bool)(LPC_GPIO2->PIN & (1U << pin_num));
    break;
  default:
    break;
  }
  return level;
}