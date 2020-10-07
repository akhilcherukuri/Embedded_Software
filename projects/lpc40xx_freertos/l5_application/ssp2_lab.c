#include "ssp2_lab.h"
#include "gpio.h"
#include "lpc40xx.h"

void ssp2__init(uint32_t max_clock_mhz) {
  // Refer to LPC User manual and setup the register bits correctly
  // a) Power on Peripheral
  const uint32_t power_on_bit = (1 << 20);
  LPC_SC->PCONP |= power_on_bit;
  // b) Setup control registers CR0 and CR1
  const uint32_t dss_bits = (0b111 << 0);
  const uint32_t sse_bit = (0b1 << 1);
  LPC_SSP2->CR0 |= dss_bits;
  LPC_SSP2->CR1 |= sse_bit;
  // c) Setup prescalar register to be <= max_clock_mhz
  const uint32_t clock = 96;
  const uint32_t prescalar_value = (clock / max_clock_mhz);
  LPC_SSP2->CPSR = prescalar_value;
}

uint8_t ssp2__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  const uint32_t busy_bit = (1 << 4);
  LPC_SSP2->DR = data_out;
  while (LPC_SSP2->SR & busy_bit) {
    ;
  }
  return LPC_SSP2->DR;
}