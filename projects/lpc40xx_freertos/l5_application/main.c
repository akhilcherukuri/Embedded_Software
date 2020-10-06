#include "ssp2_lab.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "sj2_cli.h"

#define Part_1

#ifdef Part_1

// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

void adesto_cs(void);
void adesto_ds(void);
void ssp2_pin_configuration(void);
void spi_task(void *p);
adesto_flash_id_s adesto_read_signature(void);
gpio_s adesto_external_flash_cs_signal = {GPIO__PORT_1, 10};

// TODO: Implement Adesto flash memory CS signal as a GPIO driver
void adesto_cs(void) { gpio__reset(adesto_external_flash_cs_signal); }
void adesto_ds(void) { gpio__set(adesto_external_flash_cs_signal); }

// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
adesto_flash_id_s adesto_read_signature(void) {
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
void ssp2_pin_configuration(void) {
  gpio__construct_with_function(1, 0, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 1, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 4, GPIO__FUNCTION_4);
}

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

int main(void) {
  xTaskCreate(spi_task, "SPI Part 1", 4096 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}
#endif
