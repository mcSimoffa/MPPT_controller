#include "stm8s.h"
#include <stdbool.h>
#include "pinmap.h"
#include "alive.h"
#include "adc_control.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_MIN        10
#define DUTY_MAX        180
#define DUTY_DEFAULT    (DUTY_MIN)
#define DUTY_STEP       1

#define PWM_SCALE       319     ///< 16MHz/(319+1)=50kHz PWM


// ----------------------------------------------------------------------------
static int16_t duty = DUTY_DEFAULT;

void pwm_ctrl_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_SCALE);    //16MHz/160=100kHz

  TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, duty, TIM2_OCPOLARITY_HIGH);
  TIM2_OC1PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);  ///< disable ADP3110 working via ~ODD
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Start(void)
{
  TIM2_Cmd(ENABLE);
  GPIO_WriteHigh(PWM_EN_PORT, PWM_EN_PIN);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Stop(void)
{
  GPIO_WriteLow(PWM_EN_PORT, PWM_EN_PIN);
  TIM2_Cmd(DISABLE);
}

// ----------------------------------------------------------------------------
int16_t pwm_ctrl_duty_change(uint8_t action)
{
  if (action == 'U')
  {
    if (adc_ctrl_Is_U_bat_over() == false)
    {
      duty += DUTY_STEP;
    }
    else
    {
      duty -= DUTY_STEP;
    }
  }
  else if (action == 'D')
  {
    duty -= DUTY_STEP;
  }
  else
  {
    assert_param(false);  // unknown value for action
  }

  // duty limit check
  if (duty < DUTY_MIN)
  {
    duty = DUTY_MIN;
  }
  if (duty > DUTY_MAX)
  {
    duty = DUTY_MAX;
  }

  TIM2_SetCompare1(duty);
  return duty;
}