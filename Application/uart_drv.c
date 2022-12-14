#include "uart_drv.h"
#include "ringbuf.h"

#define UART_TX_BUFFER_SIZE     192
RING_BUF_DEF(rb_uart, UART_TX_BUFFER_SIZE);

// ----------------------------------------------------------------------------
void uart_drv_Init(void)
{
    UART1_DeInit();
    UART1_Init((uint32_t)250000,
               UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE,
               UART1_MODE_TX_ENABLE);
}

// ----------------------------------------------------------------------------
void uart_drv_send(uint8_t *data, size_t size)
{
  assert_param(size);
  size_t putted = ringbufPut(&rb_uart, data, size);
  assert_param(putted == size);

  // Enable "Transmitter Enable Interrupt" and "Transmission Completes Interrupt"
    UART1->CR2 |= UART1_CR2_TIEN | UART1_CR2_TCIEN;
}


/*! ----------------------------------------------------------------------------
 * \Brief UART1 interrupt handler
*/
void uart_it_handler(void)
{
  if (UART1->SR & UART1_SR_TXE)
  {
    uint8_t charToSend;
    size_t  sizeToSend = 1;
    rb_retcode_t retcode = ringbufGet(&rb_uart, &charToSend, &sizeToSend);

    if (sizeToSend == 1)
    {
      UART1_SendData8(charToSend);
    }
    else
    {
      UART1->CR2 &= ~UART1_CR2_TIEN;  ///< Disable "Transmitter Enable Interrupt"
      if (UART1->SR & UART1_SR_TC)
      {
        // Disable "Transmitter Enable Interrupt" and Disable "Transmission Completes Interrupt"
         UART1->CR2 &= ~(UART1_CR2_TIEN | UART1_CR2_TCIEN);
      }
    }
  }
}