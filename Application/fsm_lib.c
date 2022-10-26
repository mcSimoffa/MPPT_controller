#include "stm8s_conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "sys.h"
#include "fsm_lib.h"

//----------------------------------------------------------------------
//      PRIVATE FUNCTIONS
//----------------------------------------------------------------------
static const char *getNameFromState(const FSM_t *inst, uint16_t _state)
{
  assert_param(_state < inst->total_states);
  return (inst->p_states[_state].nameState);
}


//----------------------------------------------------------------------
//      PUBLIC FUNCTIONS
//----------------------------------------------------------------------
uint16_t fsmGetState(FSM_ctx_t *ctx)
{
  return (ctx->state);
}

//-----------------------------------------------------------------------------
fsm_retcode_t fsmEnable(const FSM_t *inst, bool en, FSM_ctx_t *ctx)
{
  if (inst == NULL || ctx == NULL)
  {
    return FSM_ERROR_NULL;
  }

  if (en)
  {
  // check order of states describe. The order must be 0,1,2,3.. without jumps and back.
  uint16_t  rightOrderState = 0;
  for (uint16_t i=0; i<inst->total_states; i++)
    if (inst->p_states[i].state != rightOrderState)
    {
      assert_param(false); ///< Element has jump. Reorder state table
    }
    else
    {
      ++rightOrderState;
    }

    for (uint16_t i=0; i<inst->total_states; i++)
    {
      if (inst->p_states[i].total_signals == 0 || inst->p_states[i].exe_func == NULL)
      {
        // check parameter for macro ESM_STATES_DEF, or/and count of p_signals
        return FSM_ERROR_INVALID_DATA;
      }

      for (uint16_t j=0; j<inst->p_states[i].total_signals; j++)
      {
        if (inst->p_states[i].p_signals[j].signal == 0)
        {
          //check parameter for macro ESM_SIGNALS_DEF. Signal must be more than 0 (NO_ACTION)
          return FSM_ERROR_NOT_FOUND;
        }
      }
    }
  }
  else
  {
    ctx->state = 0;
  }

  ctx->isInit = en;
  return FSM_SUCCESS;
}


//-----------------------------------------------------------------------------
bool fsmProcess(const FSM_t *inst, FSM_ctx_t *ctx)
{
  bool retval = false;
  if (ctx->isInit)
  {
    uint16_t active_state = ctx->state;
    uint16_t i, signal;

    signal = inst->p_states[active_state].exe_func(ctx->user_ctx);

    if (signal != 0)
    {
      for (i=0; i<inst->p_states[active_state].total_signals; i++)
      {
        if (inst->p_states[active_state].p_signals[i].signal == signal)
        {
          uint16_t new_state = inst->p_states[active_state].p_signals[i].toState;
          debug_msg(LOG_COLOR_CODE_GREEN, 6,
                    "FSM[",
                    ctx->fsm_name,
                    "] sign ",
                    itoa(signal, (char *)&(char[8]){0}),
                    getNameFromState(inst, active_state),
                    getNameFromState(inst, new_state));

          if (inst->p_states[active_state].p_signals[i].f_jump)
          {
            inst->p_states[active_state].p_signals[i].f_jump(ctx->user_ctx);
          }

          ctx->state = new_state;
          retval = true;

          break;
        }
      }
      // check: action for a new state should be described
      if (i == inst->p_states[active_state].total_signals)
      {
        assert_param(false);  ///< For this state this signal not described
      }
    }
  }
  return retval;
}
