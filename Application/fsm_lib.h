#ifndef FSM_LIB_H__
#define FSM_LIB_H__

/*!
  \brief Macro to create one state with jump logic to another state
         of static allocate end state mashine
  \param [in] _state - a state which is describes
  \param [in] _exe_func - pointer to f_proc_t funcrion. This funcrion executes untin this state is sctive
  \param [in] _signals - total amount of signals. Each signal mean one possible jumo to another state
  Next all signals should be described as an array:
  of element like {SIGNAL, NEW_STATE, jump function}
  If f_p;roc function return some SIGNAL then ESM move to NEW_STATE through jump function
  */
#define FSM_STATE_DEF(_state, _str_state, _exe_func, _signals)  \
  .state = _state,                                              \
  .exe_func = _exe_func,                                        \
  .nameState = _str_state,                                      \
  .total_signals = _signals,                                    \
  .p_signals =                                                  \
  (const FSM_signal_t*)&(const FSM_signal_t[_signals])


/*!
  \brief Macro to fill a common parameters of static allocate end state mashine
  \param [in] _states - total amount of states. All explicit states should be described
                        like array of FSM_STATE_DEF
  */
#define FSM_DEF(_states)                            \
  .total_states = _states,                          \
  .p_states =                                       \
  (const FSM_state_t*)&(const FSM_state_t[_states])

typedef enum
{
  FSM_SUCCESS = 0,
  FSM_ERROR_NOT_FOUND,
  FSM_ERROR_INVALID_DATA,
  FSM_ERROR_NULL,
} fsm_retcode_t;

// context for ESM. One ESM (unified logic) can have many contexts
typedef struct
{
  void      *user_ctx;  //User context
  uint16_t  state;
  bool      isInit;
  char      *fsm_name;
  uint8_t   logLevel;   //0=None, 1=Error, 2=Warn, 3=Info, 4=Debug
} FSM_ctx_t;

typedef void      (* f_jump_t)(void *user_ctx);   //type of jump function from state to new state
typedef uint16_t  (* f_proc_t)(void *user_ctx);   //type of process function (execute when certain state is active)

typedef struct
{
  uint16_t  signal;
  uint16_t  toState;
  f_jump_t  f_jump;
} FSM_signal_t;

typedef struct
{
  uint16_t            state;
  f_proc_t            exe_func;
  char                *nameState;
  uint16_t            total_signals;
  const FSM_signal_t  *p_signals;
} FSM_state_t;


typedef struct
{
  uint16_t          total_states;
  const FSM_state_t *p_states;
} FSM_t;


/*!
  \brief Finite state machine process function
  It's a driver of end state machine. You need execute this function in main context
  \param [in] inst end state machine instance
  \return true - if this esm changed own state
          false - if the state keeps previous
  */
bool fsmProcess(const FSM_t *inst, FSM_ctx_t *ctx);


/*!
  \brief Finite state machine enable
         Finite state machine is blocked until this function unblock ESM. It also can block ESM.
  \param [in] inst  - end state machine instance
  \param [in] en    - true - enable. false- disable ESM
  \param [in] ctx   - context. This pointer will be passed to f_jump and f_proc functions

  \return FSM_SUCCESS - if OK
          FSM_ERROR_INVALID_DATA occurs if ESM_STATES_DEF(wrong), ESM_SIGNALS_DEF(0), exe function is NULL
          FSM_ERROR_NOT_FOUND occurs if ESM_SIGNALS_DEF(wrong), jump function is NULL
  */
fsm_retcode_t fsmEnable(const FSM_t *inst, bool en, FSM_ctx_t *ctx);


/*!
  \brief Get state of Finite state machine
  \param [in] ctx end state machine context
  \return current state
  */
uint16_t fsmGetState(FSM_ctx_t *ctx);

#endif	// FSM_LIB_H__