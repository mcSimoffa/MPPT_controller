#ifndef __PWM_CONTROL_H
#define __PWM_CONTROL_H

#include "stm8s.h"

void pwm_ctrl_Init(void);
void pwm_ctrl_Start(void);
bool pwm_ctrl_Enable(void);
void pwm_ctrl_Disable(void);
void pwm_en_port_int_handler(void);

#endif // __PWM_CONTROL_H
