/******************************
 *                            *
 * CMPE 244                   *
 * Lab: Watchdog-app          *
 * 10/24/2020                 *
 *                            *
 * Contributors:              *
 *                            *
 * Salvatore Nicosia          *
 * Akash Vachhani             *
 * Akhil Cherukuri            *
 *                            *
 ******************************/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "task.h"

#include "acceleration.h"
#include "ff.h"
#include "sj2_cli.h"
#include "sys_time.h"

/* Handles */
static QueueHandle_t sensor_queue;
static EventGroupHandle_t checkin;

/* Tasks */
static void producer_task(void *p);
static void consumer_task(void *p);
static void watchdog_task(void *p);

/* Helper functions */
static void write_file_using_fatfs_pi(int16_t value, const char *filename);

/* Static global variables */
static const EventBits_t bit_1 = (1 << 1);
static const EventBits_t bit_2 = (1 << 2);

int main(void) {

  sensor_queue = xQueueCreate(100, sizeof(int16_t));
  checkin = xEventGroupCreate();
  acceleration__init();

  xTaskCreate(producer_task, "producer", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer_task, "consumer", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(watchdog_task, "watchdog", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  sj2_cli__init();
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

static void producer_task(void *p) {
  int16_t average_sensor_value = 0;
  int16_t sensor_values_sum = 0;
  size_t sensor_values_count = 0;
  acceleration__axis_data_s accelerometer_axis_data = {0};

  while (1) {
    if (sensor_values_count < 100) {
      accelerometer_axis_data = acceleration__get_data();
      sensor_values_sum += accelerometer_axis_data.x;
      sensor_values_count++;
      vTaskDelay(1);
    } else {
      average_sensor_value = (sensor_values_sum / sensor_values_count);
      xQueueSend(sensor_queue, &average_sensor_value, 0);
      xEventGroupSetBits(checkin, bit_1);
      vTaskDelay(100);
    }
  }
}

static void consumer_task(void *p) {
  int16_t average_sensor_value = 0;
  uint32_t time_elapsed = sys_time__get_uptime_ms();
  const char *filename = "accelerometer_x_axis_data.csv";

  while (1) {
    if (xQueueReceive(sensor_queue, &average_sensor_value, portMAX_DELAY)) {
      xEventGroupSetBits(checkin, bit_2);
      // Open a file and append the data to an output file on the SD card every 1 sec or so
      if (sys_time__get_uptime_ms() - time_elapsed > 1000) {
        write_file_using_fatfs_pi(average_sensor_value, filename);
        time_elapsed = sys_time__get_uptime_ms();
      }
    }
  }
}

static void watchdog_task(void *p) {
  const EventBits_t wait_for_bit_1_bit_2 = bit_1 | bit_2;
  EventBits_t bits_set;

  while (1) {
    bits_set = xEventGroupWaitBits(checkin, wait_for_bit_1_bit_2, pdTRUE, pdFALSE, 200);

    if ((bits_set & (bit_1 | bit_2)) == (bit_1 | bit_2)) {
      printf("Check-in successfull from both producer and consumer task\n");
    } else if ((bits_set & bit_1) != 0) {
      printf("Check-in successfull from producer task\n");
      printf("Consumer task failed to check-in\n");
    } else {
      printf("Producer and consumer task failed to check-in within the 200ms threshold\n");
    }
    vTaskDelay(1000);
  }
}

static void write_file_using_fatfs_pi(int16_t value, const char *filename) {
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char string[64];
    sprintf(string, "%li, %i\n", xTaskGetTickCount(), value);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}
