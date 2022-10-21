#include "stm8s.h"
#include "pinmap.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_MIN        10
#define DUTY_MAX        180
#define DUTY_DEFAULT    (DUTY_MIN)
#define DUTY_STEP       1

#define PWM_SCALE       319     ///< 16MHz/(319+1)=50kHz PWM
#define TEST_V_TIMESPAN 40
// ----------------------------------------------------------------------------
void pwm_ctrl_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_SCALE);    //16MHz/160=100kHz

  TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, DUTY_DEFAULT, TIM2_OCPOLARITY_HIGH);
  TIM2_OC1PreloadConfig(ENABLE);

  TIM2_OC3Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, PWM_SCALE-TEST_V_TIMESPAN, TIM2_OCPOLARITY_LOW);
  TIM2_OC3PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);  ///< disable ADP3110 working via ~ODD
  GPIO_Init(TEST_V_PORT, TEST_V_PIN, GPIO_MODE_OUT_OD_HIZ_FAST);  ///< configure OC3 channel as OpenDrain
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Start(void)
{
  TIM2_Cmd(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Enable(void)
{
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_IN_PU_IT);
  EXTI_SetExtIntSensitivity(EXTI_PWM_EN_PORT, EXTI_SENSITIVITY_FALL_ONLY);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Disable(void)
{
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  EXTI_DeInit();
}