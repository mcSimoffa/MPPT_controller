#include "stm8s.h"
#include "pinmap.h"
#include "alive.h"
#include "adc_control.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_MIN          10
#define DUTY_MAX          100
#define DUTY_DEFAULT      (DUTY_MIN)
#define DUTY_STEP         2
#define PWM_SCALE         249     ///< 16MHz/(319+1)=50kHz PWM

// ----------------------------------------------------------------------------
static int16_t pwm_duty = DUTY_DEFAULT;
static int16_t en_duty  = DUTY_DEFAULT;

void pwm_ctrl_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_SCALE);    //16MHz/160=100kHz

  //TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, pwm_duty, TIM2_OCPOLARITY_HIGH);
  //TIM2_OC1PreloadConfig(ENABLE);

  TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, en_duty, TIM2_OCPOLARITY_HIGH);
  TIM2_OC3PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Start(void)
{
  TIM2_Cmd(ENABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Stop(void)
{
  TIM2_Cmd(DISABLE);
}

// ----------------------------------------------------------------------------
void pwm_ctrl_mode_change(bool syncroMode)
{
  if (syncroMode)
  {
    en_duty =  PWM_SCALE;
  }
  else
  {
    en_duty = pwm_duty;
  }

  TIM2_SetCompare3(en_duty);
}

// ----------------------------------------------------------------------------
int16_t pwm_ctrl_duty_change(uint8_t action)
{
  if (action == 'U')
  {
    if (adc_ctrl_Is_U_bat_over() == FALSE)
    {
      pwm_duty += DUTY_STEP;
    }
    else
    {
      pwm_duty -= DUTY_STEP;
    }
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
  if (pwm_duty < DUTY_MIN)
  {
    pwm_duty = DUTY_MIN;
  }
  if (pwm_duty > DUTY_MAX)
  {
    pwm_duty = DUTY_MAX;
  }

  TIM2_SetCompare1(pwm_duty);
  return pwm_duty;
}