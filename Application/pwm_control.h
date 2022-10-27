#ifndef __PWM_CONTROL_H
#define __PWM_CONTROL_H

#include "stm8s.h"

typedef enum
{
  PWM_STATUS_OK,
  PWM_STATUS_EMERGENCY,
} pwm_status_t;

void pwm_ctrl_Init(void);
bool pwm_ctrl_Start(void);
void pwm_ctrl_Stop(void);
pwm_status_t pwm_ctrl_Process(void);
bool pwm_ctrl_duty_change(uint8_t action);
void pwm_en_port_int_handler(void);

#endif // __PWM_CONTROL_H
