#include "alive.h"


static bool sleepLock = FALSE;
//------------------------------------------------------------------------------

void sleep_lock(void)
{
  sleepLock = TRUE;
}

//------------------------------------------------------------------------------
bool check_sleepEn()
{
  bool retval = !sleepLock;
  sleepLock = FALSE;
  return retval;
}