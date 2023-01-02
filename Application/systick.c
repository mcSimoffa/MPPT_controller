#include "stm8s.h"

#include "systick.h"

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
