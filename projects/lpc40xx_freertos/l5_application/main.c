#include "ssp2_lab.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "semphr.h"
#include "task.h"

#include "gpio.h"
#include "sj2_cli.h"

#define Part_3

// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
  // Extra Credit
  uint8_t flash_data;
} adesto_flash_id_s;

void adesto_cs(void);
void adesto_ds(void);
void ssp2_pin_configuration(void);
#ifdef Part_1
void spi_task(void *p);
#endif
#ifdef Part_2
void spi_id_verification_task1(void *p);
void spi_id_verification_task2(void *p);
SemaphoreHandle_t spi_flash_mutex;
#endif
#ifdef Part_3
void spi_task_extra(void *p);
SemaphoreHandle_t spi_flash_mutex_extra;
#endif
static adesto_flash_id_s adesto_read_signature(void);
gpio_s adesto_external_flash_cs_signal = {GPIO__PORT_1, 10};
gpio_s trigger_external_flash_cs_signal = {GPIO__PORT_1, 20};

// TODO: Implement Adesto flash memory CS signal as a GPIO driver
void adesto_cs(void) {
  gpio__reset(adesto_external_flash_cs_signal);
  gpio__set_as_output(trigger_external_flash_cs_signal);
  gpio__reset(trigger_external_flash_cs_signal);
}
void adesto_ds(void) {
  gpio__set(adesto_external_flash_cs_signal);
  gpio__set_as_output(trigger_external_flash_cs_signal);
  gpio__set(trigger_external_flash_cs_signal);
}

// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
// Part_1 or Part_2
#ifdef Part_2
static adesto_flash_id_s adesto_read_signature(void) {
  const uint8_t read_opcode = 0x9F;
  const uint8_t read_byte = 0xAB;
  adesto_flash_id_s data = {0};

  adesto_cs();
  {
    // Send opcode and read bytes
    ssp2__exchange_byte_lab(read_opcode);
    // TODO: Populate members of the 'adesto_flash_id_s' struct
    data.manufacturer_id = ssp2__exchange_byte_lab(read_byte);
    data.device_id_1 = ssp2__exchange_byte_lab(read_byte);
    data.device_id_2 = ssp2__exchange_byte_lab(read_byte);
    data.extended_device_id = ssp2__exchange_byte_lab(read_byte);
  }
  adesto_ds();

  return data;
}
#endif
void ssp2_pin_configuration(void) {
  gpio__construct_with_function(1, 0, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 1, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 4, GPIO__FUNCTION_4);
}

#ifdef Part_1
void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24;
  ssp2__init(spi_clock_mhz);

  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  //
  // Note: Configure only SCK2, MOSI2, MISO2.
  // CS will be a GPIO output pin(configure and setup direction)
  ssp2_pin_configuration();
  gpio__set_as_output(adesto_external_flash_cs_signal);

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();
    // TODO: printf the members of the 'adesto_flash_id_s' struct
    printf("Manufacturer ID %x| ", id.manufacturer_id);
    printf("Device ID1 %x| ", id.device_id_1);
    printf("Device ID2 %x| ", id.device_id_2);
    printf("Ext Device ID %x | \n", id.extended_device_id);
    vTaskDelay(500);
  }
}
#endif

#ifdef Part_2
void spi_id_verification_task1(void *p) {
  const uint32_t spi_clock_mhz = 12;
  ssp2__init(spi_clock_mhz);
  ssp2_pin_configuration();
  gpio__set_as_output(adesto_external_flash_cs_signal);
  while (1) {
    if (xSemaphoreTake(spi_flash_mutex, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();
      if (id.manufacturer_id == 0x1F) {
        printf("Task 1| ");
        printf("Manufacturer ID %x| ", id.manufacturer_id);
        printf("Device ID1 %x| ", id.device_id_1);
        printf("Device ID2 %x| ", id.device_id_2);
        printf("Ext Device ID %x | \n", id.extended_device_id);
      }
      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
      xSemaphoreGive(spi_flash_mutex);
    }
    vTaskDelay(500);
  }
}
void spi_id_verification_task2(void *p) {
  const uint32_t spi_clock_mhz = 12;
  ssp2__init(spi_clock_mhz);
  ssp2_pin_configuration();
  gpio__set_as_output(adesto_external_flash_cs_signal);
  while (1) {
    if (xSemaphoreTake(spi_flash_mutex, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();
      if (id.manufacturer_id == 0x1F) {
        printf("Task 2| ");
        printf("Manufacturer ID %x| ", id.manufacturer_id);
        printf("Device ID1 %x| ", id.device_id_1);
        printf("Device ID2 %x| ", id.device_id_2);
        printf("Ext Device ID %x | \n", id.extended_device_id);
      }
      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
      xSemaphoreGive(spi_flash_mutex);
    }
    vTaskDelay(500);
  }
}
#endif

#ifdef Part_3
/**
 * Adesto flash asks to send 24-bit address
 * We can use our usual uint32_t to store the address
 * and then transmit this address over the SPI driver
 * one byte at a time
 */
void adesto_flash_send_address(uint32_t address) {
  (void)ssp2__exchange_byte_lab((address >> 16) & 0xFF);
  (void)ssp2__exchange_byte_lab((address >> 8) & 0xFF);
  (void)ssp2__exchange_byte_lab((address >> 0) & 0xFF);
}

static adesto_flash_id_s adesto_read_manufacturer(void) {
  const uint8_t read_opcode = 0x9F;
  const uint8_t read_byte = 0xFF;
  adesto_flash_id_s data1 = {0};

  adesto_cs();
  {
    ssp2__exchange_byte_lab(read_opcode);
    data1.manufacturer_id = ssp2__exchange_byte_lab(read_byte);
  }
  adesto_ds();

  return data1;
}

static void adesto_write_enable(void) {
  // Protection Comands
  const uint8_t write_enable = 0x06;
  adesto_cs();
  { ssp2__exchange_byte_lab(write_enable); }
  adesto_ds();
}

static void adesto_page_erase(void) {
  // Page Commands
  const uint8_t page_erase = 0x20;
  const uint32_t page_address = 0x000000;
  adesto_cs();
  {
    ssp2__exchange_byte_lab(page_erase);
    adesto_flash_send_address(page_address);
  }
  adesto_ds();
  vTaskDelay(300);
}

static void adesto_register_value(void) {
  const uint8_t dummy_byte = 0xFF;
  const uint8_t status_check_register1 = 0x05;
  adesto_cs();
  {
    ssp2__exchange_byte_lab(status_check_register1);
    const uint8_t status_check_register_data = ssp2__exchange_byte_lab(dummy_byte);
    printf("Status Register value = %X | ", status_check_register_data);
  }
  adesto_ds();
}

static void adesto_data_sent(void) {
  // Page Commands
  const uint8_t page_write = 0x02;
  const uint32_t page_address = 0x000000;
  const uint8_t data_send = 0x43;
  adesto_cs();
  {
    ssp2__exchange_byte_lab(page_write);
    adesto_flash_send_address(page_address);
    ssp2__exchange_byte_lab(data_send);
    printf(" Write Data Operation: %X \n", data_send);
  }
  adesto_ds();
}

static adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};
  const uint8_t dummy_byte = 0xFF;
  const uint8_t page_read = 0x03;
  const uint32_t page_address = 0x000000;

  adesto_write_enable();

  adesto_page_erase();

  adesto_register_value();

  adesto_write_enable();

  adesto_data_sent();

  adesto_register_value();

  adesto_cs();
  {
    ssp2__exchange_byte_lab(page_read);
    adesto_flash_send_address(page_address);
    data.flash_data = ssp2__exchange_byte_lab(dummy_byte);
  }
  adesto_ds();
  return data;
}

void spi_task_extra(void *p) {
  const uint32_t spi_clock_mhz = 12;
  ssp2__init(spi_clock_mhz);
  ssp2_pin_configuration();
  gpio__set_as_output(adesto_external_flash_cs_signal);
  while (1) {
    if (xSemaphoreTake(spi_flash_mutex_extra, 1000)) {
      const adesto_flash_id_s id1 = adesto_read_manufacturer();
      if (id1.manufacturer_id == 0x1F) {
        const adesto_flash_id_s id = adesto_read_signature();
        printf(" Read Data Operation: %X \n", id.flash_data);
      }
      if (id1.manufacturer_id != 0x1F) {
        printf("Manufacturer ID read failure \n");
        vTaskSuspend(NULL);
      }
      xSemaphoreGive(spi_flash_mutex_extra);
    }
    vTaskDelay(500);
  }
}

#endif

int main(void) {

#ifdef Part_3
  spi_flash_mutex_extra = xSemaphoreCreateMutex();
  xTaskCreate(spi_task_extra, "SPI Task Extra", 4096, NULL, PRIORITY_LOW, NULL);
#endif

#ifdef Part_2
  spi_flash_mutex = xSemaphoreCreateMutex();
  xTaskCreate(spi_id_verification_task1, "SPI ID Task 1", 4096, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(spi_id_verification_task2, "SPI ID Task 2", 4096, NULL, PRIORITY_LOW, NULL);
#endif
#ifdef Part_1
  xTaskCreate(spi_task, "SPI Part 1", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}
