#ifndef __PINMAP_H
#define __PINMAP_H

#include "stm8s_gpio.h"

#define DEBUG_RXD_PIN           GPIO_PIN_6
#define DEBUG_RXD_PORT          GPIOD

#define LED_RED_PIN             GPIO_PIN_1
#define LED_RED_PORT            GPIOA

#define LED_WHITE_PIN           GPIO_PIN_2
#define LED_WHITE_PORT          GPIOA

#define DCDC_CTRL_PIN           GPIO_PIN_3
#define DCDC_CTRL_PORT          GPIOA

#define DCDC_EN_PIN             GPIO_PIN_5
#define DCDC_EN_PORT            GPIOC

#define BATT_ON_PIN             GPIO_PIN_5
#define BATT_ON_PORT            GPIOB

#define AIN_CH_BATT_VOLTAGE     ADC1_CHANNEL_2
#define BATT_VOLTAG_PIN         GPIO_PIN_4
#define BATT_VOLTAG_PORT        GPIOC

#define AIN_CH_U_SENS           ADC1_CHANNEL_3
#define U_SENS_PIN              GPIO_PIN_2
#define U_SENS_PORT             GPIOD

#define AIN_CH_I_SENS           ADC1_CHANNEL_4
#define I_SENS_PIN              GPIO_PIN_3
#define I_SENS_PORT             GPIOD

#define AIN_CH_MAX_USE          ADC1_CHANNEL_4

#endif // __PINMAP_H
