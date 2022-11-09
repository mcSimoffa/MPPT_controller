#include <string.h>
#include "stm8s.h"
#include <stdbool.h>
#include "pinmap.h"
#include "alive.h"
#include "pwm_control.h"
#include "sys.h"
#include "adc_control.h"

//-----------------------------------------------------------------------------
//   DEFINES
//-----------------------------------------------------------------------------
#define F_PRESCALER_DIV4          (0x02 << 4)
#define F_PRESCALER_DIV8          (0x04 << 4)
#define DATA_ALIGN_RIGHT          ADC1_CR2_ALIGN

#define ADC1_EOC_MASK           (1 << 7)
#define ADC1_AWD_MASK           (1 << 6)
#define TOTAL_SCANDATA_LEN      (3*sizeof(uint16_t))
#define REF_VOLTAGE_MV          1850u
#define SCALE_I                 ((uint32_t)500)
#define SCALE_U                 ((uint32_t)12500)
#define AVERAGE_BUF_SIZE        32  // should be pow of 2

#define BATTERY_HIGH_LIMIT      4200u
#define BATTERY_LOW_LIMIT       3100u
#define U_BAT_EMERGENCY         5000u
#define EMERGENCY_FOR_REF       ((1023 * REF_VOLTAGE_MV) / U_BAT_EMERGENCY)
#define DROP_CYCLES             100
//-----------------------------------------------------------------------------
//   PRIVATE TYPES
//-----------------------------------------------------------------------------
typedef struct
{
  uint16_t  ref;
  uint16_t  U;
  uint16_t  I;
} adc_data_t;

//-----------------------------------------------------------------------------
//   PRIVATE VARIABLES
//-----------------------------------------------------------------------------
static uint8_t head;
static bool isReady;
static bool isRawReady;
static bool isFrameReady;
static adc_data_t raw_data[AVERAGE_BUF_SIZE];
static adc_data_t averaged_data;

static bool emergency_flag;
adc_frame_t frame;
static uint16_t cycles_dropped;
uint8_t adc_mtx;


//-----------------------------------------------------------------------------
//   PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------
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

  cycles_dropped = 0;
  ADC1->CR1 |= ADC1_CR1_ADON;

}

// ----------------------------------------------------------------------------
void adc_ctrl_StartConv(void)
{
  ADC1->CR1 |= ADC1_CR1_ADON;
}

// ----------------------------------------------------------------------------
void adc_ctrl_Process(void)
{
  uint32_t U_bat, I_in, U_in;

  if (isRawReady)
  {
    averaging(&raw_data[0], &averaged_data);
    assert_param(averaged_data.ref > 0);      // wrong TL431 voltage = 0V

    uint32_t  ref = (uint32_t)REF_VOLTAGE_MV * 1023;
    U_bat = ref / averaged_data.ref;
    I_in = SCALE_I * averaged_data.I / averaged_data.ref;
    U_in = SCALE_U * averaged_data.U / averaged_data.ref;

    assert_param(U_bat < U16_MAX);
    assert_param(U_in < U16_MAX);
    assert_param(I_in < U16_MAX);

    frame.U_bat = (uint16_t)U_bat;
    frame.U_in = (uint16_t)U_in;
    frame.I_in = (uint16_t)I_in;

    isRawReady = FALSE;
    isFrameReady = TRUE;
    mutex_Release(&adc_mtx);
  }
}

// ----------------------------------------------------------------------------
bool  adc_ctrl_Is_Ready(void)
{
   return isReady;
}

// ----------------------------------------------------------------------------
bool adc_ctrl_Is_new_frame(void)
{
  bool retval = isFrameReady;
  isFrameReady = FALSE;
  return retval;
}

// ----------------------------------------------------------------------------
bool adc_ctrl_Is_U_bat_over(void)
{
  return (frame.U_bat > BATTERY_HIGH_LIMIT);
}

// ----------------------------------------------------------------------------
adc_frame_t *adc_ctrl_GetFrame(void)
{
  uint32_t I_bat = frame.pwr / frame.U_bat;
  assert_param(I_bat < U16_MAX);
  frame.I_bat = (uint16_t)I_bat;
  frame.pwr = frame.I_in * frame.U_in;
  frame.emergency = emergency_flag;
  return &frame;
}


/*! ----------------------------------------------------------------------------
 * \Brief ADC1 interrupt handler
*/
void adc_it_handler(void)
{
  // clear IT flags
  ADC1->CSR &= ~(ADC1_EOC_MASK | ADC1_AWD_MASK);

  if (cycles_dropped >= DROP_CYCLES)
  {
    // U_bat overvoltage checking. To preventing MCU overvoltage
    uint16_t *p_ref = (uint16_t*)&ADC1->DB2RH;
    if (*p_ref < (uint16_t)EMERGENCY_FOR_REF)
    {
      pwm_ctrl_Stop();
      emergency_flag = TRUE;
      isReady = FALSE;
      cycles_dropped = 0;
      sleep_lock();
      return;
    }

    if (mutex_tryLock(&adc_mtx))
    {
      memcpy(&raw_data[head], (uint8_t*)&ADC1->DB2RH, TOTAL_SCANDATA_LEN);

      if (++head == AVERAGE_BUF_SIZE)
      {
        head = 0;
        isReady = TRUE;
        isRawReady = TRUE;
        sleep_lock();
      }
      else
      {
        mutex_Release(&adc_mtx);
      }
    }
  }
  else
  {
    cycles_dropped++;
  }
  adc_ctrl_StartConv();
}

