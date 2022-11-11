// Header guard here
#ifndef SC_LIB_H
#define SC_LIB_H

#include <stdbool.h>

// Struct that contains all flags for function interposition
struct fn_ctrl{
  // Do we need to run a pre/post condition around fn call?
  bool sandwich_fn;
  // If non zero enter elevated, if 0 enter lowered.
  bool enter_elevated;
  // If non zero exit elevated, if 0 exit lowered.
  bool return_elevated;
  // If non zero we are shortcutting, if 0 we are not.
  bool do_shortcut;
};

#endif
