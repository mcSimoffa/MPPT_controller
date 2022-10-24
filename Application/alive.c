#include <stdbool.h>

static bool sleepLock = false;
//------------------------------------------------------------------------------

void sleep_lock(void)
{
  sleepLock = true;
}

//------------------------------------------------------------------------------
bool check_sleepEn()
{
  bool retval = !sleepLock;
  sleepLock = false;
  return retval;
}