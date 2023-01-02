#include "stm8s_conf.h"
#include <stdbool.h>
#include "pinmap.h"
#include "adc_control.h"
#include "alive.h"
#include "uart_drv.h"
#include "sys.h"
#include "systick.h"
#include "main.h"

#define PAUSE_TICKS     3
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
static uint32_t power, last_power;
static uint16_t pause = PAUSE_TICKS;
static bool clock;
//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void main(void)
{
  uint8_t cnt = 1;     // for log

  // clock start
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU


  // Init block
  uart_drv_Init();
  adc_ctrl_Init();
  systick_Init();

  /// Firstly disable DC-DC  to prevent uncontrol working
  //GPIO_Init(DCDC_EN_PORT,   DCDC_EN_PIN,   GPIO_MODE_OUT_OD_LOW_SLOW);  // LOW = prohibition
  //GPIO_Init(DCDC_CTRL_PORT, DCDC_CTRL_PIN, GPIO_MODE_OUT_OD_LOW_SLOW);
  
  // Startup block
  enableInterrupts();
  systick_Start();
  debug_msg(LOG_COLOR_CODE_BLUE, 3,"MPPT v", itoa(100, (char *)&(char[8]){0}), " starts\r\n");
  GPIO_Init(DEBUG_RXD_PORT, DEBUG_RXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);  ///< \TEST for debug

  /// Wait steady mode ADC
  bool res = adc_ctrl_StartConv();
  assert_param(res);
  while (adc_ctrl_Is_Ready() == FALSE)
  {
    adc_ctrl_Process();
  }

  /// Enable DC-DC
  GPIO_Init(DCDC_EN_PORT,   DCDC_EN_PIN,   GPIO_MODE_OUT_OD_HIZ_SLOW);  // LOW = prohibition
  GPIO_Init(DCDC_CTRL_PORT, DCDC_CTRL_PIN, GPIO_MODE_OUT_OD_HIZ_SLOW);

  // Main loop
  while (TRUE)
  {
    if (clock)
    {
      clock = FALSE;
      // conversion packet run and result wait
      adc_ctrl_Is_new_frame();  // to clear adc_ctrl_Is_new_frame() result before waiting
      bool res = adc_ctrl_StartConv();
      assert_param(res);
      do
      {
        adc_ctrl_Process();
      } while (adc_ctrl_Is_new_frame() == FALSE);

      adc_frame_t * const adc_frame = adc_ctrl_GetFrame();
      power = adc_frame->pwr;
      if (adc_frame->I_in > 700)
      {
        GPIO_WriteLow(DCDC_CTRL_PORT, DCDC_CTRL_PIN);
      }
      else if (power == 0)
      {
        GPIO_WriteHigh(DCDC_CTRL_PORT, DCDC_CTRL_PIN);
      }
      else if (power < last_power)
      {
        GPIO_WriteReverse(DCDC_CTRL_PORT, DCDC_CTRL_PIN);
      }

      // debug log
      //if (--cnt == 0)
      {
        cnt = 1;
        debug_msg(LOG_COLOR_CODE_DEFAULT, 5,
                 // "U=", itoa(adc_frame->U_in,     (char *)&(char[8]){0}),
                  " I=", itoa(adc_frame->I_in,    (char *)&(char[8]){0}),
                  " P=", itoa(power,              (char *)&(char[16]){0}),
                //  " Ub=", itoa(adc_frame->U_bat,  (char *)&(char[8]){0}),
    //              " Ib=", itoa(adc_frame->I_bat,  (char *)&(char[8]){0}),
                  "\r\n");
      }
      last_power = power;
    }
 
/// Sleep handler
    if (check_sleepEn())
    {
      //wfi();
    }
  }
}

/*! ----------------------------------------------------------------------------
 * \Brief TIM4 interrupt handler
*/
void systick_tick(void)
{
  if (--pause == 0)
  {
    pause = PAUSE_TICKS;
    clock  = TRUE;
  }
}

//-----------------------------------------------------------------------------
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  GPIO_Init(LED1_PORT,  LED1_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  while (1)
  {
    for (uint16_t i=0; i<10000; i++)
    {
      __asm("nop");
    }
    GPIO_WriteReverse(LED1_PORT,  LED1_PIN);
  }
}
#endif
