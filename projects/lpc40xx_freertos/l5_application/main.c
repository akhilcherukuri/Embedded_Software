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
#include "semphr.h"
#include "task.h"

#include "acceleration.h"
#include "ff.h"
#include "sj2_cli.h"
#include "sys_time.h"

/* Handles */
static QueueHandle_t sensor_queue;
static EventGroupHandle_t checkin;
static SemaphoreHandle_t mutex;

/* Tasks */
static void producer_task(void *p);
static void consumer_task(void *p);
static void watchdog_task(void *p);

/* Helper functions */
static void write_file_using_fatfs_pi(int16_t value, const char *filename);
static void write_error_using_fatfs_pi(char error_message[], const char *filename);

/* Static global variables */
static const EventBits_t bit_1 = (1 << 1);
static const EventBits_t bit_2 = (1 << 2);

int main(void) {

  sensor_queue = xQueueCreate(100, sizeof(int16_t));
  checkin = xEventGroupCreate();
  mutex = xSemaphoreCreateMutex();
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

  while (1) {
    if (sensor_values_count < 100) {
      sensor_values_sum += acceleration__get_data().x;
      sensor_values_count++;
      vTaskDelay(1);
    } else {
      average_sensor_value = (sensor_values_sum / sensor_values_count);
      xQueueSend(sensor_queue, &average_sensor_value, 0);
      xEventGroupSetBits(checkin, bit_1);
      sensor_values_sum = 0;
      sensor_values_count = 0;
      vTaskDelay(100);
    }
  }
}

static void consumer_task(void *p) {
  int16_t average_sensor_value = 0;
  uint32_t time_elapsed = sys_time__get_uptime_ms();
  const char *filename = "accelerometer_x_axis_data.csv";

  while (1) {
    xQueueReceive(sensor_queue, &average_sensor_value, portMAX_DELAY);
    // Open a file and append the data to an output file on the SD card every 1 sec or so
    if (sys_time__get_uptime_ms() - time_elapsed > 1000) {
      if (xSemaphoreTake(mutex, 10) == pdTRUE) {
        write_file_using_fatfs_pi(average_sensor_value, filename);
        xSemaphoreGive(mutex);
      }
      time_elapsed = sys_time__get_uptime_ms();
    }
    xEventGroupSetBits(checkin, bit_2);
  }
}

static void watchdog_task(void *p) {
  const EventBits_t wait_for_bit_1_bit_2 = bit_1 | bit_2;
  const char *filename = "accelerometer_x_axis_data.csv";
  EventBits_t bits_set;

  while (1) {
    bits_set = xEventGroupWaitBits(checkin, wait_for_bit_1_bit_2, pdTRUE, pdFALSE, 200);

    if ((bits_set & (bit_1 | bit_2)) == (bit_1 | bit_2)) {
      printf("Check-in successfull from both producer and consumer task\n");
    } else if ((bits_set & bit_1) != 0) {
      printf("Check-in successfull from producer task\n");
      printf("Consumer task failed to check-in\n");
      if (xSemaphoreTake(mutex, 10) == pdTRUE) {
        write_error_using_fatfs_pi("ERROR: Consumer task failed to check-in", filename);
        xSemaphoreGive(mutex);
      }
    } else if ((bits_set & bit_2) != 0) {
      printf("Check-in successfull from consumer task\n");
      printf("Producer task failed to check-in\n");
      if (xSemaphoreTake(mutex, 10) == pdTRUE) {
        write_error_using_fatfs_pi("ERROR: Producer task failed to check-in", filename);
        xSemaphoreGive(mutex);
      }
    } else {
      printf("Producer and Consumer task failed to check-in within the 200ms threshold\n");
      if (xSemaphoreTake(mutex, 10) == pdTRUE) {
        write_error_using_fatfs_pi("ERROR: Producer and Consumer task failed to check-in within the 200ms threshold",
                                   filename);
        xSemaphoreGive(mutex);
      }
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

static void write_error_using_fatfs_pi(char error_message[], const char *filename) {
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char error_string[128];
    sprintf(error_string, "%li, %s \n", xTaskGetTickCount(), error_message);
    if (FR_OK == f_write(&file, error_string, strlen(error_string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}