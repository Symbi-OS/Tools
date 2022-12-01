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
// TODO: ksys_ is misleading because only a subset use this entry point. Improve naming.
#define MAKE_STRUCTS(user_fn, kern_fn, fn_type) \
    typedef fn_type; \
    user_fn##_t  real_##user_fn = NULL; \
    struct fn_ctrl user_fn##_ctrl = {0, 0, 0, 0}; \
    user_fn##_t kern_fn = NULL;

 // If real_write is null, get it from dlsym
 // Place to do one time work
 // Configs ctrl struct with envt variables, gets ptrs to real and shortcut
#define MAKE_INTERPOSE_FN(user_fn, kern_fn, user_fn_sig, user_fn_call, kern_fn_call) \
    user_fn_sig { \
  if (! real_##user_fn ) { \
    get_fn_config_and_targets(& user_fn##_ctrl, (void **)&real_##user_fn, (void **) &kern_fn, \
                              #kern_fn, __func__); \
  } \
  ingress_work(&user_fn##_ctrl, __func__); \
  int ret; \
  if ( do_sc(user_fn##_ctrl.do_shortcut) ){ \
    ret = kern_fn_call \
  } else { \
    ret = user_fn_call \
  } \
  egress_work(&user_fn##_ctrl); \
  return ret;\
}

// 1) We are overriding the user function. We need to produce the signature for that fn.
// 2) We conditionally call the real function or the shortcut function, so we need
//    A) A line that calls the real function
//    or
//    B) A line that calls the shortcut function

#define USER_FN_TYPE_3(user_fn, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3) \
  ret_t (*user_fn##_t)(t_1 arg_1, t_2 arg_2, t_3 arg_3)

#define USER_FN_SIG_3(user_fn, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3) \
  ret_t user_fn(t_1 arg_1, t_2 arg_2, t_3 arg_3)

#define USER_FN_CALL_3(user_fn, arg_1, arg_2, arg_3) \
  real_##user_fn(arg_1, arg_2, arg_3);

#define KERN_FN_CALL_3(kern_fn, arg_1, arg_2, arg_3) \
  kern_fn(arg_1, arg_2, arg_3);

// #define FN_SIG_0(user_fn, ret_t) \
//   ret_t user_fn(void)
// TODO Does anyone actually use this?
#define USER_FN_TYPE_0(user_fn, ret_t) \
  ret_t (*user_fn##_t)(void)

#define USER_FN_SIG_0(user_fn, ret_t) \
  ret_t user_fn(void)

#define USER_FN_CALL_0(user_fn) \
  real_##user_fn();

#define KERN_FN_CALL_0(kern_fn) \
  kern_fn();


// Replace call to MAKE_INTERPOSE_FN_3 to use FN_TYPE_3, FN_SIG_3, FN_ARG_3

// Combine both structs, variables and fn
#define MAKE_STRUCTS_AND_FN_3(user_fn, kern_fn, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    MAKE_STRUCTS(user_fn, kern_fn, \
                 USER_FN_TYPE_3(user_fn, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3) ) \
    MAKE_INTERPOSE_FN(user_fn, kern_fn, \
                        USER_FN_SIG_3(user_fn, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ), \
                        USER_FN_CALL_3(user_fn, arg_1, arg_2, arg_3 ), \
                        KERN_FN_CALL_3(kern_fn, arg_1, arg_2, arg_3 ) )

#define MAKE_STRUCTS_AND_FN_0(user_fn, kern_fn, ret_t) \
    MAKE_STRUCTS(user_fn, kern_fn, \
                 USER_FN_TYPE_0(user_fn, ret_t) ) \
    MAKE_INTERPOSE_FN(user_fn, kern_fn, \
                        USER_FN_SIG_0(user_fn, ret_t), \
                        USER_FN_CALL_0(user_fn), \
                        KERN_FN_CALL_0(kern_fn) )
    
#endif
