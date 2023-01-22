#include "stm8s_conf.h"
#include <stdbool.h>
#include "pinmap.h"
#include "systick.h"

#include "process_indication.h"


//-----------------------------------------------------------------------------
//   PRIVATE TYPES
//-----------------------------------------------------------------------------
typedef struct
{
  uint16_t      pause;
  uint16_t      light;
} led_signals_t;

//-----------------------------------------------------------------------------
//   LOCAL VARIABLES
//-----------------------------------------------------------------------------
led_signals_t led_signals[]=
{
  {.pause = 20000, .light = 20},        // <3V3
  {.pause = 10000, .light = 20},        // 3V3 - 3V6
  {.pause = 5000, .light = 20},         // 3V6 - 3V7
  {.pause = 2000, .light = 20},         // 3V7 - 3V8
  {.pause = 1000, .light = 20},         // 3V8 - 3V9
  {.pause = 1000, .light = 500},        // 3V9 - 4V0
  {.pause = 1000, .light = 1000},       // 4V0 - 4V1
  {.pause = 1000, .light = 2000},       // 4V1 - 4V2
  {.pause = 100,  .light = 1000},       // > 4V2
  {.pause = 180,  .light = 20},         // > Charge is finished
};

uint16_t pause, light;
uint8_t index;
bool stage;
//-----------------------------------------------------------------------------
//   PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------
static void OnTimer(void)
{
  if (stage)
  {
    systic_start_timer(led_signals[index].pause, FALSE, OnTimer);
    stage = FALSE;
    GPIO_WriteHigh(LED_WHITE_PORT,  LED_WHITE_PIN);
  }
  else
  {
    systic_start_timer(led_signals[index].light, FALSE, OnTimer);
    stage = TRUE;
    GPIO_WriteLow(LED_WHITE_PORT,  LED_WHITE_PIN);
  }
}


//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void voltageIndicate_Init(void)
{
  GPIO_Init(LED_WHITE_PORT, LED_WHITE_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW); // WHITE LED off
  systic_start_timer(led_signals[index].pause, FALSE, OnTimer);
  stage = FALSE;
}


//-----------------------------------------------------------------------------
void voltageIndicate_Process(uint16_t voltage, bool isChargeFinished)
{
  if (isChargeFinished)
  {
    index = 9;
  }
  else if (voltage > 4200)
  {
    index = 8;
  }
  else if (voltage > 4100)
  {
    index = 7;
  }
  else if (voltage > 4000)
  {
    index = 6;
  }
  else if (voltage > 3900)
  {
    index = 5;
  }
  else if (voltage > 3800)
  {
    index = 4;
  }
  else if (voltage > 3700)
  {
    index = 3;
  }
  else if (voltage > 3600)
  {
    index = 2;
  }
  else if (voltage > 3300)
  {
    index = 1;
  }
  else
  {
    index = 0;
  }
}