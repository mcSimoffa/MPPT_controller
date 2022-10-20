#include "stm8s.h"
#include "pinmap.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_MIN        1
#define DUTY_MAX        180
#define DUTY_DEFAULT    (DUTY_MIN)
#define DUTY_STEP       1

#define PRESCALER_1     0
#define PWM_SCALE       319     ///< 16MHz/(319+1)=50kHz PWM

// ----------------------------------------------------------------------------
void pwm_ctrl_Init(void)
{
  TIM1_DeInit();
  TIM1_TimeBaseInit(PRESCALER_1, TIM1_COUNTERMODE_UP, PWM_SCALE, 0);    //16MHz/160=100kHz

/*
  TIM1_OCMode = TIM1_OCMODE_PWM2
  TIM1_OutputState = TIM1_OUTPUTSTATE_ENABLE
  TIM1_OutputNState = TIM1_OUTPUTNSTATE_ENABLE
  TIM1_Pulse = CCR1_Val
  TIM1_OCPolarity = TIM1_OCPOLARITY_LOW
  TIM1_OCNPolarity = TIM1_OCNPOLARITY_HIGH
  TIM1_OCIdleState = TIM1_OCIDLESTATE_SET
  TIM1_OCNIdleState = TIM1_OCIDLESTATE_RESET
  */
  TIM1_OC3Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
               DUTY_DEFAULT, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET,
               TIM1_OCNIDLESTATE_RESET);

  TIM1_SelectOutputTrigger(TIM1_TRGOSOURCE_OC3REF);     ///< Use OC3 for ADC run

  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);  ///< Init and disable signal for ADP3110
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Start(void)
{
  /* TIM1 counter enable */
  TIM1_Cmd(ENABLE);

  /* TIM1 Main Output Enable */
  TIM1_CtrlPWMOutputs(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Enable(void)
{
  GPIO_WriteHigh(PWM_EN_PORT, PWM_EN_PIN);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Disable(void)
{
  GPIO_WriteLow(PWM_EN_PORT, PWM_EN_PIN);
}