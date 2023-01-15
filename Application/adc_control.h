#ifndef __ADC_CONTROL_H
#define __ADC_CONTROL_H

#include "stm8s.h"

//-----------------------------------------------------------------------------
//   EXTERNAL TYPES
//-----------------------------------------------------------------------------
typedef struct
{
  uint32_t  pwr;
  uint16_t  U_bat;
  uint16_t  U_in;
  uint16_t  I_bat;
  uint16_t  I_in;
} adc_frame_t;

//-----------------------------------------------------------------------------
//   FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void adc_ctrl_Init(void);
bool adc_ctrl_StartConv(void);
void adc_ctrl_Process(void);

bool  adc_ctrl_Is_Ready(void);

bool adc_ctrl_Is_new_frame(void);

bool adc_ctrl_Is_U_bat_over(void);

adc_frame_t *adc_ctrl_GetFrame(void);

void adc_it_handler(void);

#endif // __ADC_CONTROL_H
