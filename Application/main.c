#include "stm8s_conf.h"
#include <stdbool.h>
#include "pinmap.h"
#include "adc_control.h"
#include "alive.h"
#include "uart_drv.h"
#include "sys.h"
#include "systick.h"
#include "pwm_control.h"
#include "main.h"

#define PWM_DUTY_ACTION_REVERSE(x)((x)^= 0x11)
#define I_BAT_LIMIT_DEFAULT             4000
#define U_BAT_LIMIT                     4100
#define MIN_U_INPUT                     6500
//-----------------------------------------------------------------------------
//   PRIVATE TYPES
//-----------------------------------------------------------------------------

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
static pwm_info_t pwm_info =
{
  .action = 'U',  ///< \U means Up = 0x55 and can be very simple transfered to \D' = 0x44 means decreases
  .terminator = 0x00,
};

static uint16_t I_bat_limit = I_BAT_LIMIT_DEFAULT;

//-----------------------------------------------------------------------------
//   PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------
static void OnTimer(void)
{
  bool res = adc_ctrl_StartConv();
  assert_param(res);
}


//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void main(void)
{
  adc_frame_t *adc_frame;
  bool isBattEn = FALSE;

  // clock start
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU

  CLK->PCKENR1 = ((1 << CLK_PERIPHERAL_UART1) | (1 << CLK_PERIPHERAL_TIMER4) | (1 << CLK_PERIPHERAL_TIMER2));
  CLK->PCKENR2 = (1 << (CLK_PERIPHERAL_ADC & 0x0F));
  
  // Init block
  uart_drv_Init();
  adc_ctrl_Init();
  pwm_ctrl_Init();
  systick_Init();

  /// Firstly start PWM  to set minamal output voltage
  pwm_ctrl_Start();
  
  // Startup block
  enableInterrupts();
  systick_Start();
  systick_delay_ms(10);
  
  debug_msg(LOG_COLOR_CODE_BLUE, 3,"MPPT v", itoa(200, (char *)&(char[8]){0}), " starts\r\n");
  GPIO_Init(DEBUG_RXD_PORT, DEBUG_RXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);  ///< \TEST for debug
    
  /// Wait steady mode ADC
  bool res = adc_ctrl_StartConv();
  assert_param(res);
  while (adc_ctrl_Is_Ready() == FALSE)
  {
    adc_ctrl_Process();
  }

  GPIO_Init(DCDC_EN_PORT,   DCDC_EN_PIN,   GPIO_MODE_OUT_PP_HIGH_SLOW); /// Enable DC-DC
  adc_ctrl_Is_new_frame();
  systic_start_timer(40, TRUE, OnTimer);
  
  // Main loop
  while (TRUE)
  {
    adc_ctrl_Process();
    if (adc_ctrl_Is_new_frame())
    {
      adc_frame = adc_ctrl_GetFrame();
      power = adc_frame->pwr;
      
      /// Preventing core working without the Solar panel
      if (adc_frame->U_in < MIN_U_INPUT)
      {
        GPIO_Init(BATT_ON_PORT,   BATT_ON_PIN,   GPIO_MODE_OUT_OD_HIZ_SLOW);
        isBattEn = FALSE;
        pwm_ctrl_Start();
      }
      else if (isBattEn == FALSE)
      {
        GPIO_Init(BATT_ON_PORT,   BATT_ON_PIN,   GPIO_MODE_OUT_OD_LOW_SLOW);
        isBattEn = TRUE;
      }
     
      
      if (isBattEn)
      {
        if (adc_frame->I_in < 15)
        {
          pwm_info.action = 'U';
        }
        else if ((adc_frame->I_bat > I_bat_limit) || (adc_frame->U_bat > U_BAT_LIMIT))
        {
          pwm_info.action = 'D';
        }
        else if (power < last_power)
        {
          PWM_DUTY_ACTION_REVERSE(pwm_info.action);
        }
        pwm_info.duty = pwm_ctrl_duty_change(pwm_info.action);
        last_power = power;
      }

      debug_msg(LOG_COLOR_CODE_DEFAULT, 13,
          "U=", itoa(adc_frame->U_in,     (char *)&(char[8]){0}),
          " I=", itoa(adc_frame->I_in,    (char *)&(char[8]){0}),
          " P=", itoa(power,              (char *)&(char[16]){0}),
          " Ub=", itoa(adc_frame->U_bat,  (char *)&(char[8]){0}),
          " Duty=", itoa(pwm_info.duty,   (char *)&(char[8]){0}),
          " Dir=", &pwm_info.action,
          "\r\n");
    }
/// Sleep handler
    if (check_sleepEn())
    {
      wfi();
    }
  }
}



//-----------------------------------------------------------------------------
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  GPIO_Init(LED1_PORT,  LED1_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  while (1)
  {
    for (uint16_t i=0; i<40000; i++)
    {
      __asm("nop");
    }
    GPIO_WriteReverse(LED1_PORT,  LED1_PIN);
  }
}
#endif
