#include "stm8s.h"
#include "pwm_control.h"
#include "adc_control.h"

void main(void)
{
  CLK_DeInit();
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);       ///< 16MHz for the peripherals
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);             ///< 16MHz for CPU
  pwm_ctrl_Init();
  adc_ctrl_Init();

  pwm_ctrl_Start();
  pwm_ctrl_Enable();
  adc_ctrl_StartConv();

  while (1)
  {
    nop();//wfi();
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
