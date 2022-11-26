#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "sc_lib.h"
#include <dlfcn.h>
#include <stdio.h>
#include <signal.h>

// TODO: Maybe move this to makefile -I?
#include "../../../Symlib/include/LINF/sym_all.h"

// Just learned this black magic
extern char **environ;

// Function that prints string in red
void print_red(const char *str) { printf("\033[1;31m%s\033[0m", str); }

// Signal handler for SIGUSR1
void sigusr1_handler(int signum) {
  printf("Received SIGUSR1\n");
  printf("lowering now\n");
  sym_lower();
}
// Signal handler for SIGUSR2
void sigusr2_handler(int signum) {
  printf("Received SIGUSR2\n");
  printf("elevating now\n");
  sym_elevate();
}

// Signal handler for SIGSYS
void sigsys_handler(int signum) {
  printf("Received SIGSYS\n");

  // Print if shortcut is on or off
  if (sc_disable) {
    printf("Shortcut is on, turning off\n");
  } else {
    printf("Shortcut is off, turning on\n");
  }
  sc_disable = !sc_disable;
}

// Function that initializes a fn_ctrl struct
void init_fn_ctrl(struct fn_ctrl *ctrl) {
  ctrl->sandwich_fn = false;
  ctrl->enter_elevated = false;
  ctrl->return_elevated = false;
  ctrl->do_shortcut = false;
}

// Returns true if the environment variable is found
bool envt_var_exists(char *var_name) {
  for (size_t i = 0; environ[i] != NULL; i++) {
    if (strncmp(environ[i], var_name, strlen(var_name)) == 0) {
      return true;
    }
  }
  return false;
}

void __attribute__((constructor)) init(void) {
  // function that is called when the library is loaded
  print_red("Library loaded\n");

  // for debugging
  // print_sc_envt_vars();

  // Register signal handlers
  signal(SIGUSR1, sigusr1_handler);
  signal(SIGUSR2, sigusr2_handler);
  signal(SIGSYS, sigsys_handler);

  // If environment variable 'BEGIN_ELE=1' is set, elevate
  if (envt_var_exists("BEGIN_ELE=1")) {
    printf("Elevating\n");
    sym_elevate();
  }

  print_red("Done initializing\n");
}

void __attribute__((destructor)) cleanUp(void) {
  // function that is called when the library is »closed«.
  sym_lower();
}



// Takes buffer, prefix, fn, and suffix
void build_envt_var(char *buf, char *prefix, const char *fn, char *suffix) {
  // Copy prefix
  strcpy(buf, prefix);
  // Copy fn
  strcat(buf, fn);
  // Copy suffix
  strcat(buf, suffix);
  printf("elevate_fn: %s\n", buf);
}

// Only works for ksys
void build_shortcut_envt_var(char *buf, char *prefix, const char *user_fn, const char *kern_fn,
                             char *suffix) {

  // Copy prefix "SHORTCUT_"
  strcpy(buf, prefix);
  // Copy fn
  strcat(buf, user_fn);
  // Copy _TO_
  strcat(buf, "_TO_");
  strcat(buf, kern_fn);
  // Copy suffix
  strcat(buf, suffix);

  printf("elevate_fn: %s\n", buf);
}

void config_fn_ctrl_for_elevate(struct fn_ctrl *ctrl) {
  // Build the environment variable
  ctrl->sandwich_fn = true;
  ctrl->enter_elevated = true;
  ctrl->return_elevated = false;
  ctrl->do_shortcut = false;
}

void config_fn_ctrl_for_lower(struct fn_ctrl *ctrl) {
  // Build the environment variable
  ctrl->sandwich_fn = true;
  ctrl->enter_elevated = false;
  ctrl->return_elevated = true;
  ctrl->do_shortcut = false;
}

void config_fn_elevate(struct fn_ctrl *ctrl, const char *fn) {
  // Build the environment variable
  char *elevate_fn = malloc(strlen("ELEVATE_") + strlen(fn) + strlen("=1") + 1);
  // check if malloc failed
  if (elevate_fn == NULL) {
    printf("Malloc failed\n");
    exit(1);
  }

  // Builds elevate_fn string "ELEVATE_fn=1"
  build_envt_var(elevate_fn, "ELEVATE_", fn, "=1");

  // If environment variable 'ELEVATE_fn=1' is set, config fn_ctrl for elevate
  if (envt_var_exists(elevate_fn)) {
    printf("Do config for elevate\n");
    config_fn_ctrl_for_elevate(ctrl);
  } else {
    // printf("no envt variable\n");
  }

  free(elevate_fn);
}

void config_fn_lower(struct fn_ctrl *ctrl, const char *fn) {

  // Manage lower case if necessary.
  char *lower_fn = malloc(strlen("LOWER_") + strlen(fn) + strlen("=1") + 1);
  build_envt_var(lower_fn, "LOWER_", fn, "=1");
  if (envt_var_exists(lower_fn)) {
    // If already sandwiching for elevate, then we can't lower
    assert(ctrl->sandwich_fn == false);

    printf("Found lower envt variable\n");
    config_fn_ctrl_for_lower(ctrl);
  } else {
    // printf("no envt variable\n");
  }
  free(lower_fn);
}

// Hard coded to ksys, make general.
void config_fn_shortcut(struct fn_ctrl *ctrl, const char *user_fn, const char *kern_fn) {
  printf("config shortcut fn is %s\n", user_fn);

  // Manage lower case if necessary.
  int len_str = strlen("SHORTCUT_") + strlen(user_fn) + strlen("_TO_") + strlen(kern_fn) + strlen("=1") + 1;
  printf("len_str is %d\n", len_str);
  char *shortcut_fn = malloc(len_str);

  if (shortcut_fn == NULL) {
    printf("Malloc failed\n");
    exit(1);
  }

  build_shortcut_envt_var(shortcut_fn, "SHORTCUT_", user_fn, kern_fn, "=1");

  if (envt_var_exists(shortcut_fn)) {
    // If already sandwiching for elevate, then we can't lower
    assert(ctrl->sandwich_fn == false);

    printf("Found shortcut envt variable\n");
    ctrl->sandwich_fn = false;
    ctrl->enter_elevated = false;
    ctrl->return_elevated = false;
    ctrl->do_shortcut = true;
  } else {
    printf("no envt variable\n");
  }

  free(shortcut_fn);
}

// Sets function control flags
void config_fn(struct fn_ctrl *ctrl, const char *fn, const char *kern_fn) {
  init_fn_ctrl(ctrl);
  // Check for elevate envt variable
  printf("Checking for elevate envt variable\n");
  // printf("envt var:%s\n", "ELEVATE" fn "=1");

  // Manage elevate case if necessary.
  config_fn_elevate(ctrl, fn);

  // Manage lower case if necessary.
  config_fn_lower(ctrl, fn);

  // Manage shortcut case if necessary.
  config_fn_shortcut(ctrl, fn, kern_fn);
}

// Print all fields of wr_ctrl struct
void print_ctrl(struct fn_ctrl *ctrl) {
  printf("ctrl->sandwich_fn: %d\n", ctrl->sandwich_fn);
  printf("ctrl->enter_elevated: %d\n", ctrl->enter_elevated);
  printf("ctrl->return_elevated: %d\n", ctrl->return_elevated);
  printf("ctrl->do_shortcut: %d\n", ctrl->do_shortcut);
}

// a function that gets called by get_fn_config_and_targets(ctrl, real_fn,
// shortcut_fn, shortcut_target, __func__);
void get_fn_config_and_targets(struct fn_ctrl *ctrl, void **real_fn,
                               void **shortcut_fn, void *shortcut_target,
                               const char *fn) {
  config_fn(ctrl, fn, shortcut_target );
  // Get address of real_write
  *real_fn = dlsym(RTLD_NEXT, fn);

  if (ctrl->do_shortcut) {
    *shortcut_fn = (void *)sym_get_fn_address(shortcut_target);

    // printf("shortcut_%s is %p \n", __func__, *shortcut_fn);

    // todo resolve this
    printf("warning: unconditionally elevating\n");
    sym_elevate();
  }
}

// TODO: assert const
void ingress_work(struct fn_ctrl *ctrl) {
  // User passed -e write, we will elevate before unconditionally
  if (ctrl->sandwich_fn) {
    if (ctrl->enter_elevated) {
      sym_elevate();
    } else {
      // User passed -l write, we will lower before unconditionally
      // printf("doing lower on entry path\n");
      sym_lower();
    }
  }
}

void egress_work(struct fn_ctrl *ctrl) {
  // User passed -e write, we will lower after unconditionally
  if (ctrl->sandwich_fn) {
    if (ctrl->return_elevated) {
      sym_elevate();
    } else {
      //  printf("doing lower on return path\n");
      sym_lower();
    }
  }
}

bool do_sc(bool do_sc_for_fn) {
  // Master control, if this isn't set, no chance we're shortcutting.
  if (!do_sc_for_fn)
    return false;
  
  // Shortcutting is enabled, we have checks to do:

  // We allow for a global shortcut disable. This is useful for interactive mode.
  if (sc_disable) 
    return false;

  // We do an intermittant normal syscall path to hit rcu etc.
  // This can be disabled for debugging
  if (intermittent_disable)
    return true;
  
  // Every Nth time should do a normal syscall path
  if ((++fn_call_ctr % MOD_VAL) == 0)
    return false;

  // If we got here, we're shortcutting
  return true;

}
// This macro allocates a "real_" fn ptr, a "ksys_" fn ptr, and a fn_ctrl struct
// Then it implements the relevant interposer fn.
MAKE_STRUCTS_AND_FN_3(write, ksys_write, ssize_t, int, fd, const void *, buf, size_t, count)
MAKE_STRUCTS_AND_FN_3(read, ksys_read, ssize_t, int, fd, void *, buf, size_t, count)

// More general, user fn, kern fn.
MAKE_STRUCTS_AND_FN_0(getpid, __do_sys_getpid, int)
