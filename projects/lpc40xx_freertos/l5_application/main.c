#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sj2_cli.h"

static QueueHandle_t queue_for_task;

void producer_task(void *p);
void consumer_task(void *p);

void producer_task(void *p) {
  const int signal_sent = 1;
  while (1) {
    xQueueSend(queue_for_task, &signal_sent, 0);

    vTaskDelay(1000);
  }
}
void consumer_task(void *p) {
  int signal_recieved;
  while (1) {
    xQueueReceive(queue_for_task, &signal_recieved, portMAX_DELAY);
    printf("Signal Recieved %d\n", signal_recieved);
  }
}

int main(void) {
  queue_for_task = xQueueCreate(1, sizeof(int));
  xTaskCreate(producer_task, "producer_task", 1024 , NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer_task, "consumer_task", 1024 , NULL, PRIORITY_LOW, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}


