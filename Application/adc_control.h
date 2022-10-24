#ifndef __ADC_CONTROL_H
#define __ADC_CONTROL_H

#include "stm8s.h"

typedef struct
{
  uint16_t  ref;
  uint16_t  U;
  uint16_t  I;
} adc_data_t;

void adc_ctrl_Init(void);
void adc_ctrl_StartConv(void);
void adc_ctrl_Process(void);
uint32_t  adc_ctrl_Get_U_bat(void);
uint32_t  adc_ctrl_Get_U_in(void);
uint32_t  adc_ctrl_Get_I_in(void);
bool adc_ctrl_is_U_bat_overvoltage(void);
bool adc_ctrl_is_U_bat_undervoltage(void);
void adc_it_handler(void);
#endif // __ADC_CONTROL_H
