#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "ring_buffer.h"
#include "uart/uart.h"

#define BAUD_RATE (115200)
#define RING_BUFFER_SIZE (64)

static ring_buffer_t rb = {0U};
static uint8_t data_buffer[RING_BUFFER_SIZE] = {0U};

// usart interrupt implementation
void usart2_isr(void) {
  const bool overrun_ocurred = usart_get_flag(USART2, USART_FLAG_ORE) == 1;
  const bool received_data = usart_get_flag(USART2, USART_FLAG_RXNE) == 1;

  if (received_data || overrun_ocurred) {
    if (ring_buffer_write(&rb, (uint8_t)usart_recv(USART2))) {
      /* Handle Failure */
    }
  }
}

void uart_setup(void) {
  ring_buffer_setup(&rb, data_buffer, RING_BUFFER_SIZE);

  rcc_periph_clock_enable(RCC_USART2);

  // old config that we dont want, I think???
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  usart_set_databits(USART2, 8);
  usart_set_baudrate(USART2, BAUD_RATE);

  // parity bit -> simple way of error detection
  // counts if there is an even number of 1s
  usart_set_parity(USART2, 0);

  usart_set_stopbits(USART2, 1);
  usart_set_mode(USART2, USART_MODE_TX_RX);

  // enable interrupts
  usart_enable_rx_interrupt(USART2);
  nvic_enable_irq(NVIC_USART2_IRQ);

  usart_enable(USART2);
}

void uart_write(uint8_t *data, const uint32_t length) {
  for (uint32_t i = 0; i < length; i++) {
    uart_write_byte(data[i]);
  }
}

void uart_write_byte(uint8_t data) {
  // send a byte and wait to complete
  usart_send_blocking(USART2, (uint16_t)data);
}

uint32_t uart_read(uint8_t *data, const uint32_t length) {
  if (length == 0) {
    return 0;
  }

  for (uint32_t i = 0; i < length; i++) {
    if (ring_buffer_read(&rb, &data[i])) {
      /* buffer is empty -> i represents num of bytes read */
      return i;
    }
  }

  return length;
}

uint8_t uart_read_byte(void) {
  uint8_t byte = 0;
  (void)ring_buffer_read(&rb, &byte);
  return byte;
}

bool uart_data_available(void) { return !ring_buffer_is_empty(&rb); }
