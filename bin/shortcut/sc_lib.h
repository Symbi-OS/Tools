// Header guard here
#ifndef SC_LIB_H
#define SC_LIB_H
// Include for ssize_t
#include <sys/types.h>

#include <stdbool.h>

// Struct that contains all flags for function interposition
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

#define MAKE_INTERPOSE_FN(fn_name, ret_t, args) \
    ret_t fn_name args { 

    void fn(){};


#endif
