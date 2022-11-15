// Header guard here
#ifndef SC_LIB_H
#define SC_LIB_H
// Include for ssize_t
#include <sys/types.h>

#include <stdbool.h>

// Struct that contains all flags for function interposition

// HACK: huge hack to toggle shortcut
int master_toggle_shortcut = 0;

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
    fn_name##_t ksys_##fn_name = NULL;

  // If real_write is null, get it from dlsym
  // Place to do one time work
  // Configs ctrl struct with envt variables, gets ptrs to real and shortcut
int hacky_ctr = 0;
#define MAKE_INTERPOSE_FN(fn_name, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    ret_t fn_name ( t_1 arg_1, t_2 arg_2, t_3 arg_3 ) { \
  if (! real_##fn_name ) { \
    get_fn_config_and_targets(& fn_name##_ctrl, (void **)&real_##fn_name, (void **) &ksys_##fn_name, \
                              "ksys_"#fn_name, __func__); \
  } \
  ingress_work(&fn_name##_ctrl); \
  int ret; \
  if (fn_name##_ctrl.do_shortcut ^ master_toggle_shortcut) { \
    if( (hacky_ctr++ % 20) == 0) { \
    ret = real_##fn_name( arg_1, arg_2, arg_3 ); \
    } else { \
    ret = ksys_##fn_name( arg_1, arg_2, arg_3 ); \
    } \
  } else { \
    ret = real_##fn_name( arg_1, arg_2, arg_3 ); \
  } \
  egress_work(&fn_name##_ctrl); \
  return ret;\
}

// Combine both structs, variables and fn
#define MAKE_STRUCTS_AND_FN(fn_name, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3 ) \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t) ( t_1 arg_1, t_2 arg_2, t_3 arg_3) ) \
    MAKE_INTERPOSE_FN(fn_name, ret_t, t_1, arg_1, t_2, arg_2, t_3, arg_3)


#endif
