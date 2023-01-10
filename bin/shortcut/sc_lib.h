// Header guard here
#ifndef SC_LIB_H
#define SC_LIB_H
// Include for ssize_t
#include <sys/types.h>

#include <stdbool.h>

// Struct that contains all flags for function interposition

// HACK: huge hack to toggle shortcut
bool sc_disable = false;
// Do normal syscall every n times
int MOD_VAL = 20;
// By default we do intermittant syscalls with the shortcutted ones.
bool intermittent_disable = false;

// Probably fine to do this globally not per fn.
unsigned int fn_call_ctr = 0;

struct fn_ctrl {
  // Do we need to run a pre/post condition around fn call?
  bool sandwich_fn;
  // If non zero enter elevated, if 0 enter lowered.
  bool enter_elevated;
  // If non zero exit elevated, if 0 exit lowered.
  bool return_elevated;
  // If non zero we are shortcutting, if 0 we are not.
  bool do_shortcut;
};

// Macro that allocates all types, variables, and structs needed
// for each function.

#define MAKE_STRUCTS(fn_name, sig) \
    typedef sig; \
    fn_name##_t  real_##fn_name = NULL; \
    struct fn_ctrl fn_name##_ctrl = {0, 0, 0, 0}; \
    fn_name##_t sc_target_##fn_name = NULL;

  // If real_write is null, get it from dlsym
  // Place to do one time work
  // Configs ctrl struct with envt variables, gets ptrs to real and shortcut
  // TODO: Could really replace __func__ with fn_name right?
#define MAKE_INTERPOSE_FN_3(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    ret_t fn_name ( t_1 arg_1, t_2 arg_2, t_3 arg_3 ) { \
  if (! real_##fn_name ) { \
    get_fn_config_and_targets(& fn_name##_ctrl, (void **)&real_##fn_name, (void **) &sc_target_##fn_name, \
                              sc_target, __func__); \
  } \
  ingress_work(&fn_name##_ctrl); \
  int ret; \
  if ( do_sc(fn_name##_ctrl.do_shortcut) ){ \
    ret = sc_target_##fn_name( arg_1, arg_2, arg_3 ); \
  } else { \
    ret = real_##fn_name( arg_1, arg_2, arg_3 ); \
  } \
  egress_work(&fn_name##_ctrl); \
  return ret;\
}

#define MAKE_INTERPOSE_FN_0(fn_name, sc_target, ret_t) \
    ret_t fn_name () { \
  if (! real_##fn_name ) { \
    get_fn_config_and_targets(& fn_name##_ctrl, (void **)&real_##fn_name, (void **) &sc_target_##fn_name, \
                              sc_target, __func__); \
  } \
  ingress_work(&fn_name##_ctrl); \
  int ret; \
  if ( do_sc(fn_name##_ctrl.do_shortcut) ){ \
    ret = sc_target_##fn_name(); \
  } else { \
    ret = real_##fn_name(); \
  } \
  egress_work(&fn_name##_ctrl); \
  return ret;\
}

// Combine both structs, variables and fn
#define MAKE_STRUCTS_AND_FN_3(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2, t_3 arg_3) ) \
    MAKE_INTERPOSE_FN_3(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3)

#define MAKE_STRUCTS_AND_FN_0(fn_name, sc_target, ret_t) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) () ) \
    MAKE_INTERPOSE_FN_0(fn_name, sc_target, ret_t)


#endif
