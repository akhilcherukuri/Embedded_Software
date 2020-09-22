#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "sj2_cli.h"

#define Part_1

#ifdef Part_1
static SemaphoreHandle_t switch_pressed_signal;
void gpio_interrupt(void);
void sleep_on_sem_task(void *p);
void configure_your_gpio_interrupt(void);
void clear_gpio_interrupt(void);

static gpio_s sw2 = {0, 30};
static gpio_s led_sw2 = {1, 24};

void configure_your_gpio_interrupt() {
  gpio__set_as_input(sw2);
  gpio__set_as_output(led_sw2);
  gpio__enable_pull_down_resistors(sw2);

  LPC_GPIOINT->IO0IntEnF |= (1 << 30);
}

void clear_gpio_interrupt() { LPC_GPIOINT->IO0IntClr |= (1 << 30); }

void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry \n");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  clear_gpio_interrupt();
  fprintf(stderr, "ISR Exit \n");
}

void sleep_on_sem_task(void *p) {
  while (1) {
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {
      delay__ms(250);
      gpio__set(led_sw2);
      delay__ms(250);
      gpio__reset(led_sw2);
    }
  }
}
#endif

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

#ifdef Part_1
  switch_pressed_signal = xSemaphoreCreateBinary();
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "gpio_interrupt");

  configure_your_gpio_interrupt();
  NVIC_EnableIRQ(GPIO_IRQn);

  xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
#endif

  return 0;
}
