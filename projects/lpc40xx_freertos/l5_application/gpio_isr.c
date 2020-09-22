#include "gpio_isr.h"
#include "lpc40xx.h"

// Part_2 for only Port 0
// extra_credit for Ports 0,1,2
#define Part_2

#ifdef Part_2
static function_pointer_t gpio0_falling_edge_callbacks[32];
static function_pointer_t gpio0_rising_edge_callbacks[32];
#endif

#ifdef extra_credit
static function_pointer_t gpios_falling_edge_callbacks[2][32];
static function_pointer_t gpios_rising_edge_callbacks[2][32];
#endif

#ifdef Part_2
void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    gpio0_falling_edge_callbacks[pin] = callback;
    LPC_GPIOINT->IO0IntEnF |= (1U << pin);
  } else {
    gpio0_rising_edge_callbacks[pin] = callback;
    LPC_GPIOINT->IO0IntEnR |= (1U << pin);
  }
}

static void clear_pin_interrupt(uint8_t pin) { LPC_GPIOINT->IO0IntClr |= (1U << pin); }

// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  uint8_t pin_that_generated_interrupt = 0;

  while ((LPC_GPIOINT->IO0IntStatF >> pin_that_generated_interrupt) && (pin_that_generated_interrupt <= 31)) {
    if (LPC_GPIOINT->IO0IntStatF >> pin_that_generated_interrupt & 1) {
      if (gpio0_falling_edge_callbacks[pin_that_generated_interrupt]) {
        function_pointer_t attached_user_handler = gpio0_falling_edge_callbacks[pin_that_generated_interrupt];
        attached_user_handler();
      }
      clear_pin_interrupt(pin_that_generated_interrupt);
    }
    pin_that_generated_interrupt++;
  }
  while ((LPC_GPIOINT->IO0IntStatR >> pin_that_generated_interrupt) && (pin_that_generated_interrupt <= 31)) {
    if (LPC_GPIOINT->IO0IntStatR >> pin_that_generated_interrupt & 1) {
      if (gpio0_rising_edge_callbacks[pin_that_generated_interrupt]) {
        function_pointer_t attached_user_handler = gpio0_rising_edge_callbacks[pin_that_generated_interrupt];
        attached_user_handler();
      }
      clear_pin_interrupt(pin_that_generated_interrupt);
    }
    pin_that_generated_interrupt++;
  }
}

#endif

#ifdef extra_credit

#endif