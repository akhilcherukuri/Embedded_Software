#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "board_io.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "sj2_cli.h"

#define Part_2

#ifdef Part_2
static SemaphoreHandle_t switch_pressed_signal;
static SemaphoreHandle_t switch_pressed_signal2;

void pin30_isr(void);
void pin29_isr(void);
void pin30_interrupt(void *p);
void pin29_interrupt(void *p);

static gpio_s led1 = {1, 24};
static gpio_s led2 = {1, 18};

void pin30_isr(void) {
  fprintf(stderr, "ISR Entry Pin 30 \n");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  fprintf(stderr, "ISR Exit Pin 30 \n");
}

void pin29_isr(void) {
  fprintf(stderr, "ISR Entry Pin 29 \n");
  xSemaphoreGiveFromISR(switch_pressed_signal2, NULL);
  fprintf(stderr, "ISR Exit Pin 29 \n");
}

void configure_your_gpio_interrupt() {
  gpio__construct_as_input(GPIO__PORT_0, 30);
  gpio__construct_as_input(GPIO__PORT_0, 29);
  gpio__construct_as_output(GPIO__PORT_1, 24);
  gpio__construct_as_output(GPIO__PORT_1, 18);
}

void pin30_interrupt(void *p) {
  while (true) {
    if (xSemaphoreTake(switch_pressed_signal, 1000)) {
      gpio__set(led1);
      vTaskDelay(500);
      gpio__reset(led1);
      vTaskDelay(500);
    }
  }
}

void pin29_interrupt(void *p) {
  while (true) {
    if (xSemaphoreTake(switch_pressed_signal2, 1000)) {
      gpio__set(led2);
      vTaskDelay(500);
      gpio__reset(led2);
      vTaskDelay(500);
    }
  }
}

#endif

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

#ifdef Part_2
  switch_pressed_signal = xSemaphoreCreateBinary();
  switch_pressed_signal2 = xSemaphoreCreateBinary();

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "gpio_interrupt");
  configure_your_gpio_interrupt();
  NVIC_EnableIRQ(GPIO_IRQn);

  gpio0__attach_interrupt(30, GPIO_INTR__RISING_EDGE, pin30_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, pin29_isr);

  xTaskCreate(pin30_interrupt, "pin30_interrupt", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(pin29_interrupt, "pin29_interrupt", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();

#endif

  return 0;
}
