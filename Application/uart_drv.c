#include "stm8s.h"
#include "ringbuf.h"
#include "uart_drv.h"


RING_BUF_DEF(rb_uart, 128);
static bool tx_busy;

// ----------------------------------------------------------------------------
void uart_drv_Init(void)
{
    UART1_DeInit();
    UART1_Init((uint32_t)115200, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
                UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TX_ENABLE);

}

// ----------------------------------------------------------------------------
void uart_drv_send(uint8_t *data, size_t size)
{
  assert_param(size);

  // first charachter put directly to UART if transmitter isn't busy.
  if (tx_busy == FALSE)
  {
    UART1_SendData8(*data);
    UART1->CR2 |= UART1_CR2_TIEN;   ///< Enable "Transmitter Enable Interrupt"
    UART1->CR2 |= UART1_CR2_TCIEN;  ///< Enable "Transmission Completes Interrupt"
    data++;
    size--;
    tx_busy = TRUE;
  }

  //  All text bytes put to ring buffer
  if (size)
  {
    size_t putted = ringbufPut(&rb_uart, data, size);
    assert_param(putted == size);
  }
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
    }
  }

  if (UART1->SR & UART1_SR_TC)
  {
     tx_busy = FALSE;
     UART1->CR2 &= ~UART1_CR2_TIEN;   ///< Disable "Transmitter Enable Interrupt"
     UART1->CR2 &= ~UART1_CR2_TCIEN;  ///< Disable "Transmission Completes Interrupt"
  }
}