#include "stm8s.h"
#include <stdbool.h>
#include "pinmap.h"
#include "adc_control.h"
#include "pwm_control.h"

// ----------------------------------------------------------------------------
#define DUTY_MIN        10
#define DUTY_MAX        180
#define DUTY_DEFAULT    (DUTY_MIN)
#define DUTY_STEP       1

#define PWM_SCALE       319     ///< 16MHz/(319+1)=50kHz PWM
#define TEST_V_TIMESPAN 40


// ----------------------------------------------------------------------------
static uint16_t duty =  DUTY_DEFAULT;
static bool batt_overvoltage;

void pwm_ctrl_Init(void)
{
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_SCALE);    //16MHz/160=100kHz

  TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, duty, TIM2_OCPOLARITY_HIGH);
  TIM2_OC1PreloadConfig(ENABLE);

  TIM2_OC3Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, PWM_SCALE-TEST_V_TIMESPAN, TIM2_OCPOLARITY_HIGH);
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
bool pwm_ctrl_Enable(void)
{
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_IN_PU_IT);
  __asm("nop");
  uint8_t enPinLevel = (GPIO_ReadInputData(PWM_EN_PORT) & PWM_EN_PIN);
  // test if EN line is kept low causes battery overvoltage
  if (enPinLevel == 0)
  {
    // turn PWM off immediately
    GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    return false;
  }

  EXTI_SetExtIntSensitivity(EXTI_PWM_EN_PORT, EXTI_SENSITIVITY_FALL_ONLY);
  return true;
}

// ----------------------------------------------------------------------------
void pwm_ctrl_Disable(void)
{
  GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  EXTI_DeInit();
}


// ----------------------------------------------------------------------------
void pwm_ctrl_Process(void)
{
  if (batt_overvoltage)
  {
    assert_param(false); ///< \TODO
  }
}
// ----------------------------------------------------------------------------
bool pwm_ctrl_duty_up(void)
{
  bool retval = false;

  if (adc_ctrl_is_U_bat_overvoltage() == false)
  {
    if (duty < DUTY_MAX + DUTY_STEP)
    {
      duty += DUTY_STEP;
      retval = true;
    }
  }
  else
  {
    duty -= DUTY_STEP;
  }

  TIM2_SetCompare1(duty);
  return retval;
}

// ----------------------------------------------------------------------------
bool pwm_ctrl_duty_down(void)
{
  bool retval = false;

  if (duty > DUTY_MIN + DUTY_STEP)
  {
    duty -= DUTY_STEP;
    retval = true;
  }

  TIM2_SetCompare1(duty);
  return retval;
}

/*! ----------------------------------------------------------------------------
 * \Brief EXTI interrupt handler for port PWM_EN_PORT
 * \Details It disable PWM driver ADP3110 immediately to preventing further overvoltage
 *           and set special flag to handle in the main context
*/
void pwm_en_port_int_handler(void)
{
  if ((GPIO_ReadInputData(PWM_EN_PORT) & PWM_EN_PIN) == 0x00)
  {
    // turn PWM off immediately
    GPIO_Init(PWM_EN_PORT, PWM_EN_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    batt_overvoltage = true;
  }
}
