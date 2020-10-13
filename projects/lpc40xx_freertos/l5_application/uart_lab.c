#include "uart_lab.h"

#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include <stdio.h>

static void uart2_register_setup(uint32_t peripheral_clock, uint32_t baud_rate);
static void uart3_register_setup(uint32_t peripheral_clock, uint32_t baud_rate);
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);
static void your_receive_interrupt(void);
void uart__enable_receive_interrupt(uart_number_e uart);
static QueueHandle_t your_uart_rx_queue;

void uart2_register_setup(uint32_t peripheral_clock, uint32_t baud_rate) {
  uint32_t enable_pcuart2 = (1 << 24);
  uint32_t wls_bit = (0x3 << 0);
  uint32_t enable_dlab = (1 << 7);
  uint16_t divider_16_bit = (peripheral_clock / (baud_rate * 16));
  uint32_t clear_all = (1 << 0) | (1 << 1) | (1 << 2);
  uint32_t set_function_2 = (1 << 1);

  LPC_SC->PCONP |= enable_pcuart2;
  LPC_UART2->LCR |= wls_bit;
  LPC_UART2->LCR |= enable_dlab;
  LPC_UART2->DLM = (divider_16_bit >> 8) & 0xFF;
  LPC_UART2->DLL = (divider_16_bit >> 0) & 0xFF;
  LPC_UART2->LCR &= ~(enable_dlab);
  LPC_IOCON->P2_8 &= ~(clear_all);
  LPC_IOCON->P2_9 &= ~(clear_all);
  LPC_IOCON->P2_8 |= set_function_2;
  LPC_IOCON->P2_9 |= set_function_2;
}

void uart3_register_setup(uint32_t peripheral_clock, uint32_t baud_rate) {
  uint32_t enable_pcuart3 = (1 << 25);
  uint32_t wls_bit = (0x3 << 0);
  uint32_t enable_dlab = (1 << 7);
  uint16_t divider_16_bit = (peripheral_clock / (baud_rate * 16));
  uint32_t clear_all = (1 << 0) | (1 << 1) | (1 << 2);
  uint32_t set_function_2 = (1 << 1);

  LPC_SC->PCONP |= enable_pcuart3;
  LPC_UART3->LCR |= wls_bit;
  LPC_UART3->LCR |= enable_dlab;
  LPC_UART3->DLM = (divider_16_bit >> 8) & 0xFF;
  LPC_UART3->DLL = (divider_16_bit >> 0) & 0xFF;
  LPC_UART3->LCR &= ~(enable_dlab);
  LPC_IOCON->P4_28 &= ~(clear_all);
  LPC_IOCON->P4_29 &= ~(clear_all);
  LPC_IOCON->P4_28 |= set_function_2;
  LPC_IOCON->P4_29 |= set_function_2;
}

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  switch (uart) {
  case UART_3:
    uart3_register_setup(peripheral_clock, baud_rate);
    break;
  case UART_2:
    uart2_register_setup(peripheral_clock, baud_rate);
    break;
  default:
    break;
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  uint8_t rdr_status = (1 << 0);
  switch (uart) {
  case UART_3:
    while (!(LPC_UART3->LSR & rdr_status)) {
      ;
    }
    *input_byte = LPC_UART3->RBR;
    break;
  case UART_2:
    while (!(LPC_UART2->LSR & rdr_status)) {
      ;
    }
    *input_byte = LPC_UART2->RBR;
    break;
  default:
    break;
  }
  return 1;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  uint8_t thre_status = (1 << 5);
  switch (uart) {
  case UART_3:
    while (!(LPC_UART3->LSR & thre_status)) {
      ;
    }
    LPC_UART3->THR = output_byte;
    while (!(LPC_UART3->LSR & thre_status)) {
      ;
    }
    break;
  case UART_2:
    while (!(LPC_UART2->LSR & thre_status)) {
      ;
    }
    LPC_UART2->THR = output_byte;
    while (!(LPC_UART2->LSR & thre_status)) {
      ;
    }
    break;
  default:
    break;
  }
  return 1;
}

static void your_receive_interrupt(void) {
  // TODO: Read the IIR register to figure out why you got interrupted
  // TODO: Based on IIR status, read the LSR register to confirm if there is data to be read
  // TODO: Based on LSR status, read the RBR register and input the data to the RX Queue
  uint8_t rdr_status = (1 << 0);
  uint8_t intid_uart3 = ((LPC_UART3->IIR & 0xE) >> 1);
  //   uint8_t intid_uart2 = ((LPC_UART2->IIR & 0xE) >> 1);
  uint8_t rda = 0x2;
  if (intid_uart3 == rda) {
    while (!(LPC_UART3->LSR & rdr_status)) {
    }
    const char byte = LPC_UART3->RBR;
    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  }
  //  if (intid_uart2 == rda) {
  //     while (!(LPC_UART2->LSR & rdr_status)) {
  //     }
  //     const char byte = LPC_UART2->RBR;
  //     xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  //   }
}

void uart__enable_receive_interrupt(uart_number_e uart) {
  // TODO: Use lpc_peripherals.h to attach your interrupt
  uint8_t receive_interrupt_enable = (1 << 0);
  // TODO: Enable UART receive interrupt by reading the LPC User manual
  // Hint: Read about the IER register
  // TODO: Create your RX queue
  switch (uart) {
  case UART_3:
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt, "ISR:UART3");
    NVIC_EnableIRQ(UART3_IRQn);
    LPC_UART3->IER |= receive_interrupt_enable;
    your_uart_rx_queue = xQueueCreate(20, sizeof(char));
    break;
  case UART_2:
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt, "ISR:UART2");
    NVIC_EnableIRQ(UART2_IRQn);
    LPC_UART2->IER |= receive_interrupt_enable;
    your_uart_rx_queue = xQueueCreate(20, sizeof(char));
    break;
  default:
    break;
  }
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}
