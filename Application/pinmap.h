#ifndef __PINMAP_H
#define __PINMAP_H

#include "stm8s_gpio.h"

#define ADC_REF_EN_PIN          GPIO_PIN_4
#define ADC_REF_EN_PORT         GPIOD

#define PWM_EN_PIN              GPIO_PIN_1
#define PWM_EN_PORT             GPIOA

#define PWM_OUT_PIN             GPIO_PIN_3
#define PWM_OUT_PORT            GPIOC

#define AIN_CH_REF_VOLTAGE      ADC1_CHANNEL_2
#define REF_VOLTAG_PIN          GPIO_PIN_4
#define REF_VOLTAG_PORT         GPIOC

#define AIN_CH_U_SENS           ADC1_CHANNEL_3
#define U_SENS_PIN              GPIO_PIN_2
#define U_SENS_PORT             GPIOD

#define AIN_CH_I_SENS           ADC1_CHANNEL_4
#define I_SENS_PIN              GPIO_PIN_3
#define I_SENS_PORT             GPIOD

#define AIN_CH_MAX_USE          ADC1_CHANNEL_4

#endif // __PINMAP_H
