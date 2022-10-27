#include <stddef.h>
#include <stdio.h>
#include "stm8s_conf.h"
#include "pwm_control.h"
#include "adc_control.h"
#include "alive.h"
#include "fsm_lib.h"
#include "uart_drv.h"
#include "sys.h"

/*! A macro for reversing 'U'->'D' or 'D'->'U'. (0x55->0x44 or 0x44->0x55)
    \param x allows only 'U' or 'D'
 */
#define PWM_DUTY_ACTION_REVERSE(x)((x)^= 0x11)


//-----------------------------------------------------------------------------
//   PRIVATE TYPES
//-----------------------------------------------------------------------------
typedef enum
{
  IDLE,
  WORKING,
  ACCIDENT,
} main_state_t;

typedef struct
{
  uint8_t action;
  uint8_t terminator; ///< it needs to debug print purpose
} pwm_duty_t;

//-----------------------------------------------------------------------------
//   LOCAL VARIABLES
//-----------------------------------------------------------------------------
static const uint8_t  helloMsg[] = "MPPT starts\r\nv";
static main_state_t main_state;
static pwm_duty_t pwm_duty =
{
  .action = 'U',  ///< \U means Up = 0x55 and can be very simple transfered to \D' = 0x44 means decreases
  .terminator = 0x00,
};

static uint32_t power, last_power;
static uint16_t work_cyc;

//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void main(void)
{
  // clock start
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU

  // Init block
  uart_drv_Init();
  pwm_ctrl_Init();
  adc_ctrl_Init();

  // Startup block
  enableInterrupts();
  debug_msg(LOG_COLOR_CODE_BLUE, 2,helloMsg, itoa(100, (char *)&(char[8]){0}));
  adc_ctrl_StartConv(); ///< To voltages validate validate

  // Main loop
  while (TRUE)
  {
    adc_ctrl_Process();
    if (pwm_ctrl_Process() == PWM_STATUS_EMERGENCY)
    {
      main_state = ACCIDENT;
      debug_msg(LOG_COLOR_CODE_RED, 1, "->ACCIDENT");
    }

    /// State machine handler
    switch (main_state)
    {
      case IDLE:
        if (adc_ctrl_is_Ready() && (adc_ctrl_is_U_bat_over() == FALSE))
        {
          if (pwm_ctrl_Start())
          {
            main_state = WORKING;
            debug_msg(LOG_COLOR_CODE_GREEN, 1, "->WORKING");
            adc_ctrl_set_SysTick(50);
            sleep_lock();
          }
        }
        break;

    case WORKING:
      if (adc_ctrl_is_newTick())
      {
        power = adc_ctrl_getPower();
        if (power < last_power)
        {
          PWM_DUTY_ACTION_REVERSE(pwm_duty.action);  // reverse duty action 0x55->0x44->0x55
        }
        debug_msg(LOG_COLOR_CODE_DEFAULT, 4, "cyc ", itoa(work_cyc, (char *)&(char[8]){0}), " dir=", &pwm_duty.action);
        pwm_ctrl_duty_change(pwm_duty.action);    /// \TODO false return isn't handled yet
        last_power = power;
      }
      break;

    case ACCIDENT:
      break;    /// \TODO

    default:
      assert_param(false);
    }

/// Sleep handler
    if (check_sleepEn())
    {
      //wfi();
    }
  }
}



#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
