#ifndef __PWM_CONTROL_H
#define __PWM_CONTROL_H

#include "stm8s.h"

//-----------------------------------------------------------------------------
//   EXTERNAL TYPES
//-----------------------------------------------------------------------------
typedef enum
{
  PWM_STATUS_OK,
  PWM_STATUS_EMERGENCY,
} pwm_status_t;


//-----------------------------------------------------------------------------
//   FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void pwm_ctrl_Init(void);
void pwm_ctrl_Start(void);
void pwm_ctrl_Stop(void);

int16_t pwm_ctrl_duty_change(uint8_t action);

#endif // __PWM_CONTROL_H
