#include "stm8s.h"
#include <stdlib.h>

#include "systick.h"

#define SYSTICK_CB_MAX_COUNT    3

typedef struct
{
  uint16_t      period;
  uint16_t      cnt;
  bool          repeat;
  systick_cb_t  cb;
} cb_t;

static uint16_t pause;
static cb_t     cbs[SYSTICK_CB_MAX_COUNT];

// ----------------------------------------------------------------------------
void systick_Init(void)
{
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);     //1ms
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

}

// ----------------------------------------------------------------------------
void systick_Start(void)
{
  TIM4_Cmd(ENABLE);
}

// ----------------------------------------------------------------------------
void systick_Stop(void)
{
  TIM4_Cmd(DISABLE);
}

// ----------------------------------------------------------------------------
void systick_delay_ms(uint16_t ticks)
{
  pause = ticks;
  while (pause)
  {
    wfi();
  }
}

// ----------------------------------------------------------------------------
void systic_start_timer(uint16_t ticks, bool repeat, systick_cb_t cb)
{
  assert_param(cb);
  for (uint8_t i=0; i<SYSTICK_CB_MAX_COUNT; i++)
  {
    if (cbs[i].cb == NULL)
    {
      cbs[i].period = ticks;
      cbs[i].cnt = ticks;
      cbs[i].repeat = repeat;
      cbs[i].cb = cb;
      return;
    }
  }
  assert_param(FALSE);
}

// ----------------------------------------------------------------------------
void systic_stop_timer(systick_cb_t cb)
{
  assert_param(cb);
  for (uint8_t i=0; i<SYSTICK_CB_MAX_COUNT; i++)
  {
    if (cbs[i].cb == cb)
    {
      cbs[i].cb = NULL;
      return;
    }
  }
  assert_param(FALSE);
}

/*! ----------------------------------------------------------------------------
 * \Brief TIM4 interrupt handler
*/
void systick_tick(void)
{
  if (pause)
  {
    pause--;
  }
  
  for (uint8_t i=0; i<SYSTICK_CB_MAX_COUNT; i++)
  {
    if (cbs[i].cb)
    {
      if (--cbs[i].cnt == 0)
      {
        cbs[i].cb();
        if (cbs[i].repeat)
        {
          cbs[i].cnt = cbs[i].period;
        }
        else
        {
          cbs[i].cb = NULL;
        }
      }
    }
  }
}
