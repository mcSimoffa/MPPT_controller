#include "stm8s.h"
#include "pinmap.h"
#include "alive.h"
#include "main.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_STEP         1
#define PWM_SCALE         319     ///< 16MHz/(319+1)=50kHz PWM
#define DUTY_MAX          PWM_SCALE
#define PWM_DUTY_MIN      100

// ----------------------------------------------------------------------------
static int16_t pwm_duty = 0;

// ----------------------------------------------------------------------------
void pwm_ctrl_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_SCALE);

  TIM2_OC3Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, pwm_duty, TIM2_OCPOLARITY_HIGH);
  TIM2_OC3PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Start(void)
{
  pwm_duty = PWM_DUTY_MIN;
  TIM2_SetCompare3(pwm_duty);
  TIM2_Cmd(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Stop(void)
{
  TIM2_Cmd(DISABLE);
}


// ----------------------------------------------------------------------------
int16_t pwm_ctrl_duty_change(uint8_t action)
{
  if (action == 'U')
  {
      pwm_duty += DUTY_STEP;
  }
  else if (action == 'D')
  {
    pwm_duty -= DUTY_STEP;
  }
  else
  {
    assert_param(FALSE);  // unknown value for action
  }

  // pwm_duty limit check
  pwm_duty = MIN(pwm_duty, DUTY_MAX);
  pwm_duty = MAX(pwm_duty, PWM_DUTY_MIN);

  TIM2_SetCompare3(pwm_duty);
  return pwm_duty;
}