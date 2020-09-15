#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "gpio_lab.h"
#include "lpc40xx.h"
#include "sj2_cli.h"

// part_1 , part_2, part_3, easter_egg
#define part_3

#ifdef part_3
static SemaphoreHandle_t switch_press_indication;
#endif

typedef struct {
  uint8_t port;
  uint8_t pin;
} port_pin_s;

#ifdef part_1
// PART 1
static void led_task1(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  /* Set the IOCON MUX function select pins to 000 */
  LPC_IOCON->P1_18 &= ~(7U << 0);

  /* Set the DIR register bit for the LED port pin */
  const uint32_t sjtwo_led0 = (1U << 18);
  LPC_GPIO1->DIR |= sjtwo_led0;

  while (true) {
    /* Set PIN register bit to 0 to turn ON LED (led may be active low) */
    LPC_GPIO1->CLR = sjtwo_led0;
    vTaskDelay(200);

    /* Set PIN register bit to 1 to turn OFF LED */
    LPC_GPIO1->SET = sjtwo_led0;
    vTaskDelay(200);
  }
}
#endif

#ifdef part_2
// PART 2
static void led_task2(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  const port_pin_s *led = (port_pin_s *)(task_parameter);

  while (true) {
    gpiolab__set_high(led->pin, led->port);
    vTaskDelay(100);

    gpiolab__set_low(led->pin, led->port);
    vTaskDelay(100);
  }
}
#endif

#ifdef part_3
// PART 3
static void led_task(void *task_parameter) {
  const port_pin_s *led = (port_pin_s *)task_parameter;

  while (true) {
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      gpiolab__set_low(led->pin, led->port);
      vTaskDelay(100);
      gpiolab__set_high(led->pin, led->port);
      vTaskDelay(100);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}

static void switch_task(void *task_parameter) {
  const port_pin_s *sw = (port_pin_s *)task_parameter;

  while (true) {
    if (gpiolab__get_level(sw->pin, sw->port)) {
      while (gpiolab__get_level(sw->pin, sw->port)) {
        vTaskDelay(100);
      }
      xSemaphoreGive(switch_press_indication);
    }
    vTaskDelay(100);
  }
}
#endif

#ifdef easter_egg
// Easter EGG
#endif

int main(void) {

#ifdef part_1
  xTaskCreate(led_task1, "led0", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#ifdef part_2
  static port_pin_s led0 = {1, 18};
  static port_pin_s led1 = {1, 24};
  xTaskCreate(led_task2, "led0", 2048 / sizeof(void *), (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(led_task2, "led1", 2048 / sizeof(void *), (void *)&led1, PRIORITY_LOW, NULL);
#endif

#ifdef part_3
  switch_press_indication = xSemaphoreCreateBinary();
  static port_pin_s sjtwo_switch = {1, 15};
  static port_pin_s sjtwo_led2 = {1, 26};
 
  xTaskCreate(led_task, "led3", 1024, &sjtwo_led2, PRIORITY_LOW, NULL);
  xTaskCreate(switch_task, "switch", 1024, &sjtwo_switch, PRIORITY_LOW, NULL);
#endif

#ifdef easter_egg
#endif

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
