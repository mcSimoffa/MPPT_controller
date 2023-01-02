#ifndef __SYSTICKL_H
#define __SYSTICKL_H

#include "stm8s.h"

typedef void(*systick_cb_t)(void);

//-----------------------------------------------------------------------------
//   FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void systick_Init(void);

void systick_Start(void);

void systick_Stop(void);

void systick_tick(void);

void systic_start_timer(uint16_t ticks, bool repeat, systick_cb_t cb);

void systic_stop_timer(systick_cb_t cb);

void systick_delay_ms(uint16_t ticks);

#endif // __SYSTICKL_H
