#include <string.h>
#include "stm8s.h"
#include "pinmap.h"
#include "alive.h"
#include "adc_control.h"

//-----------------------------------------------------------------------------
//   DEFINES
//-----------------------------------------------------------------------------
#define F_PRESCALER_DIV4          (0x02 << 4)
#define F_PRESCALER_DIV8          (0x04 << 4)
#define DATA_ALIGN_RIGHT          ADC1_CR2_ALIGN
//#define EXT_EVENT_TIM1_TRGO       (0x00 << 4)

#if AWD_IN_USE
#define HIGH_AWD_THR_DEFAULT      (uint16_t)853   ///< it is 2.5V level in the full ADC range 0-3V
#define LOW_AWD_THR_DEFAULT       (uint16_t)602   ///< it is 2.5V level in the full ADC range 0-4.25
#endif

#define ADC1_EOC_MASK           (1 << 7)
#define ADC1_AWD_MASK           (1 << 6)
#define TOTAL_SCANDATA_LEN      (3*sizeof(uint16_t))
#define TL431_VOLTAGE_MV        ((uint32_t)2500)
#define SCALE_I                 ((uint32_t)500)
#define SCALE_U                 ((uint32_t)12500)
#define AVERAGE_BUF_SIZE        8  // should be pow of 2
#define BATTERY_HIGH_LIMIT      4200
#define BATTERY_LOW_LIMIT       3100

//-----------------------------------------------------------------------------
//   PRIVATE VARIABLES
//-----------------------------------------------------------------------------
static uint8_t head;
static bool ready_flag;
static adc_data_t raw_data[AVERAGE_BUF_SIZE];
static adc_data_t averaged_data;
static bool newSample;
static uint32_t U_bat, I_in, U_in;
 //-----------------------------------------------------------------------------
//   PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
/*! ----------------------------------------------------------------------------
 * \Brief Prepring ADC module for scanning CH0 to CH4
          Conversion starts via an external signal: TIM1 OC3
          Analog watchDog watches CH2
*/
void adc_ctrl_Init(void)
{
  // Configure ADC related GPIOs
  GPIO_Init(REF_VOLTAG_PORT,  REF_VOLTAG_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(U_SENS_PORT,      U_SENS_PIN,     GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(I_SENS_PORT,      I_SENS_PIN,     GPIO_MODE_IN_FL_NO_IT);

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

  // f+ADC=16/8=2MHz
  ADC1->CR1 = (uint8_t)(F_PRESCALER_DIV8);  ///< Test cont mode | ADC1_CR1_CONT

#if AWD_IN_USE
  // Set default thrsholds to watch power supply voltage in the range 3-4.25V
  ADC1_SetHighThreshold(HIGH_AWD_THR_DEFAULT);
  ADC1_SetLowThreshold(LOW_AWD_THR_DEFAULT);

  //Enable Analog WatchDog for the 2.5V rererence voltage channel
  ADC1->AWCRL = (uint8_t)(1 << AIN_CH_REF_VOLTAGE);
  ADC1->AWCRH = (uint8_t)0x00;
#endif

  ADC1->CR1 |= ADC1_CR1_ADON;
}

// ----------------------------------------------------------------------------
void adc_ctrl_StartConv(void)
{
  ADC1->CR1 |= ADC1_CR1_ADON;
}

static void averaging(const adc_data_t *raw, adc_data_t *avg)
{
  assert_param(raw);
  assert_param(avg);
  memset(avg, 0, sizeof(adc_data_t));
  for (uint8_t i=0; i<AVERAGE_BUF_SIZE; i++)
  {
    avg->I += raw->I;
    avg->U += raw->U;
    avg->ref += raw->ref;
  }
  avg->I /= AVERAGE_BUF_SIZE;
  avg->U /= AVERAGE_BUF_SIZE;
  avg->ref /= AVERAGE_BUF_SIZE;
}

// ----------------------------------------------------------------------------
void adc_ctrl_Process(void)
{
  uint32_t volatile U_bat, I_in, U_in;

  if (newSample)
  {
    newSample = FALSE;
    if (ready_flag)
    {
      averaging(&raw_data[0], &averaged_data);
      if (averaged_data.ref == 0)
      {
         assert_param(FALSE); // wrong TL431 voltage = 0V
      }
      else
      {
        uint32_t  ref = TL431_VOLTAGE_MV * 1023;
        U_bat = ref / averaged_data.ref;
        I_in = SCALE_I * averaged_data.I / averaged_data.ref;
        U_in = SCALE_U * averaged_data.U / averaged_data.ref;
      }
    }
    adc_ctrl_StartConv();
  }
}

// ----------------------------------------------------------------------------
uint32_t  adc_ctrl_Get_U_bat(void)
{
   return U_bat;
}

// ----------------------------------------------------------------------------
uint32_t  adc_ctrl_Get_U_in(void)
{
   return U_in;
}

// ----------------------------------------------------------------------------
uint32_t  adc_ctrl_Get_I_in(void)
{
   return I_in;
}

// ----------------------------------------------------------------------------
bool adc_ctrl_is_U_bat_overvoltage(void)
{
  return (U_bat > BATTERY_HIGH_LIMIT);
}

// ----------------------------------------------------------------------------
bool adc_ctrl_is_U_bat_undervoltage(void)
{
  return (U_bat <BATTERY_LOW_LIMIT);
}

/*! ----------------------------------------------------------------------------
 * \Brief ADC1 interrupt handler
*/
void adc_it_handler(void)
{
  // The reading sequence isn't accorded to rm0016 ch28.4. The OVR flag also isn't checked
  // The EOC flag isn't checked because only EOC interrupt is enabled
  memcpy(&raw_data[head++], (uint8_t*)&ADC1->DB2RH, TOTAL_SCANDATA_LEN);
  if ((ready_flag == FALSE) && (head == AVERAGE_BUF_SIZE))
  {
    ready_flag = TRUE;
  }
  head %= AVERAGE_BUF_SIZE;
  newSample = true;
  ADC1->CSR &= ~(ADC1_EOC_MASK | ADC1_AWD_MASK);

  //! \FEATURE this registor got to zero together with CSR_AWD
  // ADC1->AWSRL &= ~(1 << AIN_CH_REF_VOLTAGE);
  sleep_lock();
}

