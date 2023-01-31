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

// Make function signatures, needs types and args
#define MAKE_SIG(ret_t, fn_name, ...) \
    ret_t fn_name ( __VA_ARGS__ )
// Make function call, needs args
#define MAKE_FN_CALL(fn_name, ...) \
    fn_name ( __VA_ARGS__ )

#define MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, types_and_args, args) \
    MAKE_SIG(ret_t, fn_name, types_and_args) { \
  if (! real_##fn_name ) { \
    get_fn_config_and_targets(& fn_name##_ctrl, (void **)&real_##fn_name, (void **) &sc_target_##fn_name, \
                              sc_target, __func__); \
  } \
  ingress_work(&fn_name##_ctrl); \
  ret_t ret; \
  if ( do_sc(fn_name##_ctrl.do_shortcut) ){ \
    ret = MAKE_FN_CALL(sc_target_##fn_name, args); \
  } else { \
    ret = MAKE_FN_CALL(real_##fn_name, args); \
  } \
  egress_work(&fn_name##_ctrl); \
  return ret;\
}

// This just turns n args into a single arg.
#define COMBINE_ARGS(...) __VA_ARGS__

#define MAKE_STRUCTS_AND_FN_6(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3, t_4, arg_4, t_5, arg_5, t_6, arg_6) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, t_5 arg_5, t_6 arg_6) ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, t_5 arg_5, t_6 arg_6), \
                      COMBINE_ARGS(arg_1, arg_2, arg_3, arg_4, arg_5, arg_6) )
// Do this for 5
#define MAKE_STRUCTS_AND_FN_5(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3, t_4, arg_4, t_5, arg_5) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, t_5 arg_5) ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, t_5 arg_5), \
                      COMBINE_ARGS(arg_1, arg_2, arg_3, arg_4, arg_5) )

// Combine both structs, variables and fn
#define MAKE_STRUCTS_AND_FN_3(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2, t_3 arg_3) ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3), \
                      COMBINE_ARGS(arg_1, arg_2, arg_3) )

#define MAKE_STRUCTS_AND_FN_2(fn_name, sc_target, ret_t, t_1, arg_1, t_2, arg_2) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2) ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, COMBINE_ARGS(t_1 arg_1, t_2 arg_2), COMBINE_ARGS(arg_1, arg_2) )

#define MAKE_STRUCTS_AND_FN_1(fn_name, sc_target, ret_t, t_1, arg_1) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1) ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, COMBINE_ARGS(t_1 arg_1), COMBINE_ARGS(arg_1) )

#define MAKE_STRUCTS_AND_FN_0(fn_name, sc_target, ret_t) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) () ) \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, COMBINE_ARGS(), COMBINE_ARGS())


#endif
