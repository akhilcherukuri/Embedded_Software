/*
Explanation and Screenshot are in PDF in given Merge Request.

Additional Questions:
Q1) What is the purpose of the block time during xQueueReceive()?
A1) The max amount of time(in ticks) to wait for queue to receive data.

Q2) What if you use ZERO block time during xQueueReceive()?
A2) If no data is received in queue by function call, continues forward to task completion.
*/

#include <stdio.h>

#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "queue.h"
#include "task.h"

#include "sj2_cli.h"

static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

void producer(void *p);
void consumer(void *p);
switch_e get_switch_input_from_switch0(void);

int main(void) {

  puts("Starting RTOS");
  // TODO: Create your tasks
  xTaskCreate(producer, "producer", 4096, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "consumer", 4096, NULL, PRIORITY_LOW, NULL);

  // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    const switch_e switch_value = get_switch_input_from_switch0();

    // TODO: Print a message before xQueueSend()
    printf("Producer Task \n");
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    xQueueSend(switch_queue, &switch_value, 0);
    // TODO: Print a message after xQueueSend()
    printf("Line %d, sending switch status: %i \n", __LINE__, switch_value);
    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    printf("Cosumer Task \n");
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    // TODO: Print a message after xQueueReceive()
    printf("Line %d, receving switch status: %i \n", __LINE__, switch_value);
  }
}

switch_e get_switch_input_from_switch0(void) {
  uint32_t switch0 = (1 << 29);
  LPC_GPIO0->DIR &= ~(1 << 29);
  if (LPC_GPIO0->PIN & switch0) {
    return switch__on;
  } else {
    return switch__off;
  }
}