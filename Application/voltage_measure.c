#include "stm8s.h"
#include "adc_control.h"


void adc_test(void)
{
  GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_FL_NO_IT);

  ADC1_DeInit();

  ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS, ADC1_CHANNEL_4, ADC1_PRESSEL_FCPU_D2, \
            ADC1_EXTTRIG_TIM, DISABLE, ADC1_ALIGN_RIGHT, ADC1_SCHMITTTRIG_CHANNEL4,\
            DISABLE);

  ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);

  enableInterrupts();

  ADC1_StartConversion();
}

