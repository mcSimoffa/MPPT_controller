#ifndef ALIVE_H
#define ALIVE_H
#include <stdbool.h>
/*!
 * \brief This API needs to prevent suddenly sleep until all events not handled
 * When some event occurs at the end of main loop application needs to one more main cycle to habdle this event
 */
 
/*!
 * \brief Postpone enter to light sleep mode for one main loop pass.
 * A good way to invoke this function always when any ESM changed own state
 * or one or more events occurs. Because application must handled it before light sleep enter
 */
void sleep_lock(void);

/*!
 * \brief Check light sleep postpone status
 * Light sleep enter should only if return true.
 */
bool check_sleepEn(void);

#endif // SYS_ALIVE_H
