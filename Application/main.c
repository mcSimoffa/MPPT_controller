#include "stm8s_conf.h"
#include <stdbool.h>
#include "pinmap.h"
#include "adc_control.h"
#include "alive.h"
#include "uart_drv.h"
#include "sys.h"
#include "systick.h"
#include "pwm_control.h"
#include "process_indication.h"

#include "main.h"

#define PWM_DUTY_ACTION_REVERSE(x)((x)^= 0x11)
#define U_BAT_LIMIT                     4210
#define U_BAT_NOT_FULL                  4100        
#define I_BAT_FUUL_CHARGE               160
#define CRITICAL_U_INPUT                6500
#define NOMINAL_U_INPUT                 8000
#define MIN_REGULATION_CURRENT          5       // mA
#define INPUT_CURRENT_LIMIT             700     // To preventing  DC-DC damage
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
static uint32_t last_power;
static pwm_info_t pwm_info =
{
  .action = 'U',  ///< \U means Up = 0x55 and can be very simple transfered to \D' = 0x44 means decreases
  .terminator = 0x00,
};

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
  bool isFullCharged = FALSE;
  bool isFirstFrame = TRUE;
  uint16_t questionCurrent = 0;
  
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
  systick_delay_ms(100);
  
  debug_msg(LOG_COLOR_CODE_BLUE, 3,"MPPT v", itoa(200, (char *)&(char[8]){0}), " starts\r\n");
  GPIO_Init(DEBUG_RXD_PORT, DEBUG_RXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);  ///< \TEST for debug
  GPIO_Init(BATT_ON_PORT,   BATT_ON_PIN,   GPIO_MODE_OUT_OD_HIZ_SLOW);
  GPIO_Init(LED_RED_PORT,   LED_RED_PIN,   GPIO_MODE_OUT_PP_HIGH_SLOW);
  voltageIndicate_Init();
  
  /// Wait steady mode ADC
  bool res = adc_ctrl_StartConv();
  assert_param(res);
  while (adc_ctrl_Is_Ready() == FALSE)
  {
    adc_ctrl_Process();
  }
  
  GPIO_Init(DCDC_EN_PORT,   DCDC_EN_PIN,   GPIO_MODE_OUT_PP_HIGH_SLOW); /// Enable DC-DC
  adc_ctrl_Is_new_frame();
  systic_start_timer(50, TRUE, OnTimer);
  
  // Main loop
  while (TRUE)
  {
    adc_ctrl_Process();
    if (adc_ctrl_Is_new_frame())
    {
      adc_frame = adc_ctrl_GetFrame();
      if (isFirstFrame)
      {
        questionCurrent = adc_frame->I_in;
        questionCurrent += MIN_REGULATION_CURRENT;   // Add a threshold to be confident: the regulation process is activated
        isFirstFrame = FALSE;
      }
      
      /// Preventing core working without the Solar panel
      if ((adc_frame->U_in < CRITICAL_U_INPUT) && isBattEn)
      {
        GPIO_WriteHigh(BATT_ON_PORT,   BATT_ON_PIN);
        isBattEn = FALSE;
        pwm_ctrl_Start();
      }
      else if (isBattEn == FALSE)
      {
        GPIO_WriteLow(BATT_ON_PORT,   BATT_ON_PIN);
        isBattEn = TRUE;
      }
     
      if (isFullCharged)
      {
        //GPIO_WriteReverse(LED_WHITE_PORT,  LED_WHITE_PIN);  
        if (adc_frame->U_bat < U_BAT_NOT_FULL)
        {
          isFullCharged = FALSE;
          pwm_ctrl_Start();
          GPIO_WriteHigh(DCDC_EN_PORT,   DCDC_EN_PIN);
        }
      }
      else if (isBattEn)
      {
        if ((adc_frame->I_in > INPUT_CURRENT_LIMIT) || (adc_frame->U_in < NOMINAL_U_INPUT))
        {
          pwm_info.action = 'D';
        }
        else if (adc_frame->I_in < questionCurrent)
        {
          pwm_info.action = 'U';
        }
        else if (adc_frame->U_bat > U_BAT_LIMIT)
        {
          if (adc_frame->I_bat < I_BAT_FUUL_CHARGE)
          {
            isFullCharged = TRUE;
            pwm_ctrl_Stop();
            GPIO_WriteLow(DCDC_EN_PORT,   DCDC_EN_PIN);
            
          }
          pwm_info.action = 'D';
        }
        else if (adc_frame->pwr < last_power)
        {
          PWM_DUTY_ACTION_REVERSE(pwm_info.action);
        }
        
        pwm_info.duty = pwm_ctrl_duty_change(pwm_info.action);
        last_power = adc_frame->pwr;
      }
      
      voltageIndicate_Process(adc_frame->U_bat, isFullCharged);
      
      debug_msg(LOG_COLOR_CODE_DEFAULT, 13,
          "U=", itoa(adc_frame->U_in,     (char *)&(char[8]){0}),
          " I=", itoa(adc_frame->I_in,    (char *)&(char[8]){0}),
          " P=", itoa(adc_frame->pwr,              (char *)&(char[16]){0}),
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
  disableInterrupts();
  GPIO_Init(LED_WHITE_PORT, LED_WHITE_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW); // WHITE LED off
  while (TRUE)
  {
    for (uint16_t i=0; i<40000; i++)
    {
      __asm("nop");
    }
    GPIO_WriteReverse(LED_RED_PORT,  LED_RED_PIN);
  }
}
#endif
