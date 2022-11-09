#ifndef __SYSTICKL_H
#define __SYSTICKL_H

#include "stm8s.h"


//-----------------------------------------------------------------------------
//   FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void systick_Init(void);
void systick_Start(void);
void systick_Stop(void);

#endif // __SYSTICKL_H
