#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "clock.h"
#include "gpio.h"
#include "sj2_cli.h"
#include "uart_lab.h"
#include <stdlib.h>
#include <string.h>

#define Part_3_Receiver

#ifdef Part_0_1
void uart_read_task(void *p);
void uart_write_task(void *p);
const uint32_t peripheral_clock = (96 * 1000 * 1000);

void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char read_byte;
    uart_lab__polled_get(UART_3, &read_byte);
    printf("Byte Received: %c \n", read_byte);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    char send_byte = 'A';
    uart_lab__polled_put(UART_3, send_byte);
    printf("Byte Trasmitted: %c \n", send_byte);
    vTaskDelay(500);
  }
}
#endif

#ifdef Part_2
void uart_read_task2(void *p);
void uart_write_task2(void *p);
const uint32_t peripheral_clock = (96 * 1000 * 1000);

void uart_read_task2(void *p) {
  while (1) {
    char read_byte;
    uart__enable_receive_interrupt(UART_3);
    uart_lab__get_char_from_queue(&read_byte, 1000);
    printf("Byte Received From Interrupt: %c \n", read_byte);
    vTaskDelay(500);
  }
}

void uart_write_task2(void *p) {
  while (1) {
    char send_byte = 'A';
    uart_lab__polled_put(UART_3, send_byte);
    printf("Byte Trasmitted: %c \n", send_byte);
    vTaskDelay(500);
  }
}
#endif

#ifdef Part_3_Sender
void board_1_sender_task(void *p);
const uint32_t peripheral_clock = (96 * 1000 * 1000);
// This task is done for you, but you should understand what this code is doing
void board_1_sender_task(void *p) {
  char number_as_string[16] = {0};

  while (true) {
    const int number = rand();
    sprintf(number_as_string, "%i", number);

    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }

    printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}
#endif

#ifdef Part_3_Receiver
void board_2_receiver_task(void *p);
const uint32_t peripheral_clock = (96 * 1000 * 1000);

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received: %c\n", byte);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this number from the other board: %s\n", number_as_string);
    }
    // We have not yet received the NULL '\0' char, so buffer the data
    else {
      // TODO: Store data to number_as_string[] array one char at a time
      // Hint: Use counter as an index, and increment it as long as we do not reach max value of 16
      for (counter = 0; counter < 16; counter++) {
        number_as_string[counter] = byte;
      }
    }
  }
}
#endif

int main(void) {
  // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
#ifdef Part_0_1
  uart_lab__init(UART_3, peripheral_clock, 9600);
  xTaskCreate(uart_read_task, "uart_read_task", 4096, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "uart_write_task", 4096, NULL, PRIORITY_LOW, NULL);
#endif

#ifdef Part_2
  uart_lab__init(UART_3, peripheral_clock, 9600);
  xTaskCreate(uart_read_task2, "uart_read_task2", 4096, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task2, "uart_write_task2", 4096, NULL, PRIORITY_LOW, NULL);
#endif

#ifdef Part_3_Sender
  uart_lab__init(UART_3, peripheral_clock, 9600);
  xTaskCreate(board_1_sender_task, "board_1_sender_task", 4096, NULL, PRIORITY_LOW, NULL);
#endif

#ifdef Part_3_Receiver
  uart_lab__init(UART_3, peripheral_clock, 9600);
  xTaskCreate(board_2_receiver_task, "board_2_receiver_task", 4096, NULL, PRIORITY_LOW, NULL);
#endif

  puts("Starting RTOS");
  vTaskStartScheduler();
  return 0;
}
