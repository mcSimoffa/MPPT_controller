#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "uart_drv.h"
#include "sys.h"

//----------------------------------------------------------------------
//      PRIVATE VARIABLES
//----------------------------------------------------------------------
#if defined(UART_LOG_ENABLED) && UART_LOG_ENABLED && defined (LOG_USES_COLORS) && LOG_USES_COLORS
static const char * m_colors[] =
{
  "\x1B[0m",
  "\x1B[1;30m",
  "\x1B[1;31m",
  "\x1B[1;32m",
  "\x1B[1;33m",
  "\x1B[1;34m",
  "\x1B[1;35m",
  "\x1B[1;36m",
  "\x1B[1;37m",
};
#endif

//------------------------------------------------------------
char *itoa(int32_t n, char *buf)
{
  char c;   //for reverse
  bool sign = (bool)(n < 0);
  char *s = buf;

  // construct string in the reverse mode
  *s++ = '\0'; ///< null terminator is first

  if (sign)
  {
    n = -n;
  }

  do
  {
    *s++ = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign)
  {
    *s++ = '-';
  }

  // set pointers before string reverse
  s--;    ///< end of string pointer
  char *p_s = buf;  ///< start of string

  do
  {
    c = *p_s;
    *p_s++ = *s;
    *s-- = c;
  } while (s > p_s);

  return buf;
}


//-----------------------------------------------------------------------------
#if defined(UART_LOG_ENABLED) && UART_LOG_ENABLED
void debug_msg(uint8_t color_id, uint8_t params, ...)
{
  uint8_t*  param;

  if (params == 0)
  {
    return;
  }

#if defined (LOG_USES_COLORS) && LOG_USES_COLORS
  if (color_id)
  {
    uart_drv_send((uint8_t*)m_colors[color_id], strlen(m_colors[color_id]));
  }
#endif

  va_list args;
  va_start (args, params);
  for (uint32_t i=0; i<params; i++)
  {
    param = va_arg(args, uint8_t*);
    uart_drv_send(param, strlen((char const*)param));
  }
  va_end(args);

#if defined (LOG_USES_COLORS) && LOG_USES_COLORS
  if (color_id)
  {
    uart_drv_send((uint8_t*)m_colors[0], strlen(m_colors[0]));
  }
#endif
}
#else //UART_LOG_ENABLED
void debug_msg(uint8_t color_id, uint8_t params, ...) {}
#endif //UART_LOG_ENABLED
