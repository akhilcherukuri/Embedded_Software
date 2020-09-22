#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "sj2_cli.h"

#define Part_0

#ifdef Part_0
void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry \n");
  LPC_GPIOINT->IO0IntClr |= (1 << 30);
  fprintf(stderr, "ISR Exit \n");
}
#endif

int main(void) {

#ifdef Part_0
  static gpio_s sw2 = {0, 30};
  static gpio_s led_sw2 = {1, 24};
  gpio__set_as_input(sw2);
  gpio__set_as_output(led_sw2);
  gpio__enable_pull_down_resistors(sw2);

  LPC_GPIOINT->IO0IntEnF |= (1 << 30);

  NVIC_EnableIRQ(GPIO_IRQn);

  while (1) {
    delay__ms(250);
    gpio__set(led_sw2);
    delay__ms(250);
    gpio__reset(led_sw2);
  }
#endif

  return 0;
}
