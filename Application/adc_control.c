#include "stm8s.h"
#include "pinmap.h"
#include "adc_control.h"

// ----------------------------------------------------------------------------
#define F_PRESCALER_DIV2          0x00
#define DATA_ALIGN_RIGHT          ADC1_CR2_ALIGN
#define EXT_EVENT_TIM1_TRGO       (0x00 << 4)
#define HIGH_AWD_THR_DEFAULT      (uint16_t)853   ///< it is 2.5V level in the full ADC range 0-3V
#define LOW_AWD_THR_DEFAULT       (uint16_t)602   ///< it is 2.5V level in the full ADC range 0-4.25

uint16_t adc_val[3];

/*! ----------------------------------------------------------------------------
 * \Brief Prepring ADC module for scanning CH0 to CH4
          Conversion starts via an external signal: TIM1 OC3
          Analog watchDog watches CH2
*/
void adc_ctrl_Init(void)
{
  // Configure ADC related GPIOs
  GPIO_Init(REF_VOLTAG_PORT, REF_VOLTAG_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(U_SENS_PORT, U_SENS_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(I_SENS_PORT, I_SENS_PIN, GPIO_MODE_IN_FL_NO_IT);

  GPIO_Init(ADC_REF_EN_PORT, ADC_REF_EN_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);  ///< Reference voltage supplier enable
  GPIO_Init(DEBUG_RXD_PORT, DEBUG_RXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);  ///< \TEST for debug

  ADC1_DeInit();

  // Disable a Shmidt trigger for 3 channels in use
  ADC1->TDRL = (uint8_t)0x0;
  ADC1->TDRH = (uint8_t)((0x01 << AIN_CH_REF_VOLTAGE) | (0x01 << AIN_CH_U_SENS) |(0x01 << AIN_CH_I_SENS));

  //Data align right, external event from TIM1_TRGO
  ADC1->CR2 = (uint8_t)(ADC1_CR2_SCAN | DATA_ALIGN_RIGHT);///< Test start from TIM1 | EXT_EVENT_TIM1_TRGO | ADC1_CR2_EXTTRIG

  // Enable End Of Conversion interrupt  and Analog WatchDog interrupt
  ADC1->CSR = (uint8_t)(AIN_CH_MAX_USE | ADC1_CSR_EOCIE);  ///> \Note: ADC1_CSR_AWDIE interferes a scan process

  // f+ADC=16/2=8MHz
  ADC1->CR1 = (uint8_t)(F_PRESCALER_DIV2);  ///< Test cont mode | ADC1_CR1_CONT

  // Set default thrsholds to watch power supply voltage in the range 3-4.25V
  ADC1_SetHighThreshold(HIGH_AWD_THR_DEFAULT);
  ADC1_SetLowThreshold(LOW_AWD_THR_DEFAULT);

  //Enable Analog WatchDog for the 2.5V rererence voltage channel
  ADC1->AWCRL = (uint8_t)(1 << AIN_CH_REF_VOLTAGE);
  ADC1->AWCRH = (uint8_t)0x00;

  ADC1->CR1 |= ADC1_CR1_ADON;
  enableInterrupts();
}

void adc_ctrl_StartConv(void)
{
  ADC1->CR1 |= ADC1_CR1_ADON;
}

