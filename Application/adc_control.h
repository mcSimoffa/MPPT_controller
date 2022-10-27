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

void adc_ctrl_set_SysTick(uint16_t period);
bool adc_ctrl_is_newTick(void);

bool  adc_ctrl_is_Ready(void);
bool adc_ctrl_is_U_bat_over(void);
uint32_t adc_ctrl_getPower(void);

void adc_it_handler(void);

#endif // __ADC_CONTROL_H
