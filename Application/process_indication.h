#ifndef PROCESS_INDICATION_H_
#define PROCESS_INDICATION_H_

#include "stm8s.h"

void voltageIndicate_Init(void);
void voltageIndicate_Process(uint16_t voltage, bool isChargeFinished);

#endif // PROCESS_INDICATION_H_