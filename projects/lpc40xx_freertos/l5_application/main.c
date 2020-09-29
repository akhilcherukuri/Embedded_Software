#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "pwm1.h"

/// Change to Part_0, Part_1, Part_2
#define Part_0

#ifdef Part_0

void pin_configure_pwm_channel_as_io_pin() { gpio__construct_with_function(2, 0, GPIO__FUNCTION_1); }

void pwm_task(void *p) {
  pwm1__init_single_edge(1000);

  // Locate a GPIO pin that a PWM channel will control
  // NOTE You can use gpio__construct_with_function() API from gpio.h
  // TODO Write this function yourself
  pin_configure_pwm_channel_as_io_pin();

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  pwm1__set_duty_cycle(PWM1__2_0, 50);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, percent);

    if (++percent > 100) {
      percent = 0;
    }

    vTaskDelay(100);
  }
}

#endif

int main(void) {

  xTaskCreate(pwm_task, "pwm_task", 1024, NULL, PRIORITY_LOW, NULL);
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}
