#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "adc.h"
#include "gpio.h"

/// Change to Part_0, Part_1, Part_2
#define Part_1

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

#ifdef Part_1

void pin_configure_adc_channel_as_io_pin() { gpio__construct_with_function(GPIO__PORT_0, 25, GPIO__FUNCTION_1); }

void adc_task(void *p) {
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  pin_configure_adc_channel_as_io_pin(); // TODO You need to write this function

  while (1) {
    // Get the ADC reading using a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
    fprintf(stderr, "ADC reading: %d\n", adc_value);
    vTaskDelay(100);
  }
}

#endif

int main(void) {

#ifdef Part_1
  xTaskCreate(adc_task, "adc_task", 1024, NULL, PRIORITY_LOW, NULL);
#endif

#ifdef Part_0
  xTaskCreate(pwm_task, "pwm_task", 1024, NULL, PRIORITY_LOW, NULL);
#endif

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}
