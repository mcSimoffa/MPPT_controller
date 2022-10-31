#include <stddef.h>
#include <stdio.h>
#include "stm8s_conf.h"
#include "pinmap.h"
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
  STEPPING,
  GET_STALE,
  ACCIDENT,
} main_state_t;

typedef struct
{
  uint8_t action;
  uint8_t terminator; ///< it needs to debug print purpose
  uint16_t duty;
} pwm_info_t;

//-----------------------------------------------------------------------------
//   LOCAL VARIABLES
//-----------------------------------------------------------------------------
static main_state_t main_state;
static pwm_info_t pwm_info =
{
  .action = 'U',  ///< \U means Up = 0x55 and can be very simple transfered to \D' = 0x44 means decreases
  .terminator = 0x00,
};

static uint32_t power, last_power;

//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void main(void)
{
  // clock start
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU

  GPIO_Init(DEBUG_RXD_PORT, DEBUG_RXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);  ///< \TEST for debug

  // Init block
  uart_drv_Init();
  pwm_ctrl_Init();
  adc_ctrl_Init();

  // Startup block
  enableInterrupts();
  debug_msg(LOG_COLOR_CODE_BLUE, 3,"MPPT v", itoa(100, (char *)&(char[8]){0}), " starts\r\n");
  adc_ctrl_StartConv(); ///< To voltages validate validate

  // Main loop
  while (TRUE)
  {
    adc_ctrl_Process();

    /// State machine handler
    switch (main_state)
    {
      case IDLE:
        if (adc_ctrl_Is_Ready() && (adc_ctrl_Is_U_bat_over() == FALSE))
        {
          pwm_ctrl_Start();
          main_state = STEPPING;
          debug_msg(LOG_COLOR_CODE_GREEN, 1, "->STEPPING\r\n");
          sleep_lock();
        }
        break;

    case STEPPING:
      if (adc_ctrl_Is_new_frame())
      {
        adc_frame_t * const adc_frame = adc_ctrl_GetFrame();
        if (adc_frame->emergency)
        {
          main_state = ACCIDENT;
          debug_msg(LOG_COLOR_CODE_RED, 1, "->ACCIDENT\r\n");
          break;
        }
        power = adc_frame->pwr;
        if (power < last_power)
        {
          PWM_DUTY_ACTION_REVERSE(pwm_info.action);  // reverse duty action 0x55->0x44->0x55
        }
        pwm_info.duty = pwm_ctrl_duty_change(pwm_info.action);

        // debug log
        debug_msg(LOG_COLOR_CODE_DEFAULT, 15,
                  "U=", itoa(adc_frame->U_in,     (char *)&(char[8]){0}),
                  " I=", itoa(adc_frame->I_in,    (char *)&(char[8]){0}),
                  " P=", itoa(power,              (char *)&(char[16]){0}),
                  " Ub=", itoa(adc_frame->U_bat,  (char *)&(char[8]){0}),
                  " Ib=", itoa(adc_frame->I_bat,  (char *)&(char[8]){0}),
                  " Duty=", itoa(pwm_info.duty,   (char *)&(char[8]){0}),
                  " Dir=", &pwm_info.action,
                  "\r\n");
        last_power = power;
        main_state = GET_STALE;
      }
      break;

    case GET_STALE:
      {
        static uint16_t stale_cnt = 0;
        if (adc_ctrl_Is_new_frame())
        {
          if (++stale_cnt == 100)
          {
            stale_cnt = 0;
            main_state = STEPPING;
          }
        }
        break;
      }

    case ACCIDENT:
      assert_param(false);  /// \TODO
      break;

    default:
      assert_param(false);
    }

/// Sleep handler
    if (check_sleepEn())
    {
      wfi();
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
