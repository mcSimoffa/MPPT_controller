#ifndef _SYS_H_
#define _SYS_H_

#include "stm8s.h"

#define LOG_COLOR_CODE_DEFAULT 0
#define LOG_COLOR_CODE_BLACK   1
#define LOG_COLOR_CODE_RED     2
#define LOG_COLOR_CODE_GREEN   3
#define LOG_COLOR_CODE_YELLOW  4
#define LOG_COLOR_CODE_BLUE    5
#define LOG_COLOR_CODE_MAGENTA 6
#define LOG_COLOR_CODE_CYAN    7
#define LOG_COLOR_CODE_WHITE   8

char *itoa(int16_t n, char *buf);

void debug_msg(uint8_t color_id, uint8_t params, ...);

#endif // _SYS_H_