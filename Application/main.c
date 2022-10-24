#include "stm8s_conf.h"
#include "pwm_control.h"
#include "adc_control.h"
#include "alive.h"
#include "fsm_lib.h"
#include <stddef.h>

//-----------------------------------------------------------------------------
//   PRIVATE TYPES
//-----------------------------------------------------------------------------
typedef enum
{
  STATE_IDLE,
  STATE_ADC_START,
  STATE_PWM_STAB,
  STATES_TOTAL,  //states count
} state_t;

typedef enum
{
  SIGNAL_NO_ACTION = 0,
  SIGNAL_SOLAR_READY,
  SIGNAL_REF_READY,
  SIGNAL_TO,
  SIGNAL_OVERVOLTAGE,
  SIGNAL_CHRGE_COMPLETED,
  SIGNAL_WEAK_SOLAR,
} signal_t;

//-----------------------------------------------------------------------------
//   PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------
static uint16_t  idleProc(void * ctx)
{
  return SIGNAL_NO_ACTION;
}

//-----------------------------------------------------------------------------
static void startADC(void * ctx)
{

}

//-----------------------------------------------------------------------------
static uint16_t  waitADC(void * ctx)
{
  return SIGNAL_NO_ACTION;
}

//-----------------------------------------------------------------------------
static void startPWM(void * ctx)
{

}

//-----------------------------------------------------------------------------
static void turnOf(void * ctx)
{

}

//-----------------------------------------------------------------------------
static uint16_t  trackingPower(void * ctx)
{
  return SIGNAL_NO_ACTION;
}

//-----------------------------------------------------------------------------
static void pwmAction(void * ctx)
{

}

//-----------------------------------------------------------------------------
static void alarm(void * ctx)
{

}
//-----------------------------------------------------------------------------
//   PRIVATE VARIABLES
//-----------------------------------------------------------------------------
static const FSM_t main_fsm =
{
  FSM_DEF(STATES_TOTAL)
  {
    {
      FSM_STATE_DEF(STATE_IDLE, "IDLE", idleProc, 1)
          { {SIGNAL_SOLAR_READY,   STATE_ADC_START,     startADC}  }
    },
    {
      FSM_STATE_DEF(STATE_ADC_START, "ADC_START", waitADC, 2)
          { {SIGNAL_REF_READY,   STATE_PWM_STAB,  startPWM},
            {SIGNAL_WEAK_SOLAR,  STATE_IDLE,      turnOf } }
    },
    {
      FSM_STATE_DEF(STATE_PWM_STAB, "PWM_STAB", trackingPower, 4)
          { {SIGNAL_TO,               STATE_PWM_STAB, pwmAction},
            {SIGNAL_OVERVOLTAGE,      STATE_IDLE,     alarm},
            {SIGNAL_WEAK_SOLAR,       STATE_IDLE,     turnOf},
            {SIGNAL_CHRGE_COMPLETED,  STATE_IDLE,     turnOf }  }
    },
  }
};

static FSM_ctx_t       fsm_ctx;

//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void main(void)
{
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU

  // Main finite state machine init
  fsm_ctx.logLevel = 0;
  fsm_ctx.fsm_name = "MAIN";
  assert_param(fsmGetState(&fsm_ctx) == STATE_IDLE);
  fsm_retcode_t ret_code = fsmEnable(&main_fsm, TRUE, &fsm_ctx);
  assert_param(ret_code == FSM_SUCCESS);

  // Init block
  pwm_ctrl_Init();
  adc_ctrl_Init();

  // Startup block
  adc_ctrl_StartConv(); ///< To voltages validate validate

  while (TRUE)
  {
    adc_ctrl_Process();

    if (check_sleepEn())
    {
      wfi();
    }
  }

  pwm_ctrl_Start();
  pwm_ctrl_Enable();
  adc_ctrl_StartConv();


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
