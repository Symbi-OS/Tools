#define _GNU_SOURCE

#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <stdio.h>
#include <dlfcn.h>
#include "sc_lib.h"

// TODO: Maybe move this to makefile -I?
#include "../../../Symlib/include/LINF/sym_all.h"

// Just learned this black magic
extern char** environ;

// Function that prints string in red
void print_red(const char* str){
    printf("\033[1;31m%s\033[0m", str);
}

#if 0
void print_sc_envt_vars(){
  // Produce array of environment variables
  for (size_t i = 0; environ[i] != NULL; i++) {
    // ELEVATE
    if (strncmp(environ[i], "ELEVATE", 7) == 0) {
      // Print the environment variable, skipping the first 8 chars and removing the last 2 chars
      printf("%s\n", environ[i]);
      // printf("%s\n", environ[i] + 8);
      // If the environment variable suffix is 'read=1' set the read fn
      // if(strncmp(environ[i] + 8, "read=1", 6) == 0){
      //   printf("Need to elevate %s\n", environ[i] + 8);
      //  //rd_ctrl.do_shortcut = true;

      // }
    }
    // LOWER
    if (strncmp(environ[i], "LOWER", 5) == 0) {
      printf("%s\n", environ[i]);
    }
    // SHORTCUT
    if (strncmp(environ[i], "SHORTCUT", 8) == 0) {
      printf("%s\n", environ[i]);
    }
  }

}
#endif

// Function that initializes a fn_ctrl struct
void init_fn_ctrl(struct fn_ctrl* ctrl){
  ctrl->sandwich_fn = false;
  ctrl->enter_elevated = false;
  ctrl->return_elevated = false;
  ctrl->do_shortcut = false;
}

// Returns true if the environment variable is found
bool envt_var_exists(char *var_name){
  for (size_t i = 0; environ[i] != NULL; i++) {
    if (strncmp(environ[i], var_name, strlen(var_name)) == 0) {
      return true;
    }
  }
  return false;
}

void __attribute__ ((constructor)) init(void) {
  // function that is called when the library is loaded
  print_red("Library loaded\n");

  // for debugging
  // print_sc_envt_vars();

  // If environment variable 'BEGIN_ELE=1' is set, elevate
  if (envt_var_exists("BEGIN_ELE=1")) {
    printf("Elevating\n");
    sym_elevate();
  }

  print_red("Done initializing\n");

}

void __attribute__ ((destructor)) cleanUp(void) {
  // function that is called when the library is »closed«.
  sym_lower();
}

// Fn declarations for interposition.
typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);

// Add to these

// Where the calls would have gone to before us.
read_t real_read = NULL;
write_t real_write = NULL;

// Function control structs
struct fn_ctrl write_ctrl;
struct fn_ctrl read_ctrl;

// Pointers to shortcut function targets.
write_t ksys_write = NULL;
read_t ksys_read = NULL;


// Takes buffer, prefix, fn, and suffix
void build_envt_var(char * buf, char * prefix, const char * fn, char * suffix){
    // Copy prefix
    strcpy(buf, prefix);
    // Copy fn
    strcat(buf, fn);
    // Copy suffix
    strcat(buf, suffix);
    printf("elevate_fn: %s\n", buf);
}

// Only works for ksys
void build_shortcut_envt_var(char * buf, char * prefix, const char * fn, char * suffix){

    // Copy prefix "SHORTCUT_"
    strcpy(buf, prefix);
    // Copy fn
    strcat(buf, fn);
    // Copy _TO_
    strcat(buf, "_TO_");
    strcat(buf, "ksys_");
    strcat(buf, fn);
    // Copy suffix
    strcat(buf, suffix);

    printf("elevate_fn: %s\n", buf);
}

void config_fn_ctrl_for_elevate(struct fn_ctrl * ctrl){
  // Build the environment variable
    ctrl->sandwich_fn = true;
    ctrl->enter_elevated = true;
    ctrl->return_elevated = false;
    ctrl->do_shortcut = false;
}

void config_fn_ctrl_for_lower(struct fn_ctrl * ctrl){
  // Build the environment variable
    ctrl->sandwich_fn = true;
    ctrl->enter_elevated = false;
    ctrl->return_elevated = true;
    ctrl->do_shortcut = false;
}

// void config_fn_for_shortcut(struct fn_ctrl * ctrl){
//   // Build the environment variable
//     ctrl->sandwich_fn = true;
//     ctrl->enter_elevated = false;
//     ctrl->return_elevated = false;
//     ctrl->do_shortcut = true;
// }

void config_fn_elevate(struct fn_ctrl * ctrl, const char * fn){
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
  if(envt_var_exists(elevate_fn)){
    printf("Do config for elevate\n");
    config_fn_ctrl_for_elevate(ctrl);
  }else{
    // printf("no envt variable\n");
  }

  free(elevate_fn);
}

void config_fn_lower(struct fn_ctrl *ctrl, const char* fn){

  // Manage lower case if necessary.
  char *lower_fn = malloc(strlen("LOWER_") + strlen(fn) + strlen("=1") + 1);
  build_envt_var(lower_fn, "LOWER_", fn, "=1");
  if(envt_var_exists(lower_fn)){
    // If already sandwiching for elevate, then we can't lower
    assert(ctrl->sandwich_fn == false);

    printf("Found lower envt variable\n");
    config_fn_ctrl_for_lower(ctrl);
  }else{
    // printf("no envt variable\n");
  }
  free(lower_fn);
}

void config_fn_shortcut(struct fn_ctrl *ctrl, const char* fn){
  printf("config shortcut fn is %s\n", fn);

  // Manage lower case if necessary.
  char *shortcut_fn = malloc(strlen("SHORTCUT_") + strlen(fn) + \
                              strlen("_TO_") + strlen("ksys_") + \
                              strlen(fn) + \
                              strlen("=1") + 1);

  build_shortcut_envt_var(shortcut_fn, "SHORTCUT_", fn, "=1");

  if(envt_var_exists(shortcut_fn)){
    // If already sandwiching for elevate, then we can't lower
    assert(ctrl->sandwich_fn == false);

    printf("Found shortcut envt variable\n");
    ctrl->sandwich_fn = false;
    ctrl->enter_elevated = false;
    ctrl->return_elevated = false;
    ctrl->do_shortcut = true;
  }else{
    printf("no envt variable\n");
  }

  free(shortcut_fn);
}


// Sets function control flags
void config_fn(struct fn_ctrl *ctrl, const char *fn){
  init_fn_ctrl(ctrl);
  // Check for elevate envt variable
  printf("Checking for elevate envt variable\n");
  // printf("envt var:%s\n", "ELEVATE" fn "=1");

  // Manage elevate case if necessary.
  config_fn_elevate(ctrl, fn);

  // Manage lower case if necessary.
  config_fn_lower(ctrl, fn);

  // Manage shortcut case if necessary.
  config_fn_shortcut(ctrl, fn);

}

// Print all fields of wr_ctrl struct
void print_ctrl(struct fn_ctrl *ctrl){
  printf("ctrl->sandwich_fn: %d\n", ctrl->sandwich_fn);
  printf("ctrl->enter_elevated: %d\n", ctrl->enter_elevated);
  printf("ctrl->return_elevated: %d\n", ctrl->return_elevated);
  printf("ctrl->do_shortcut: %d\n", ctrl->do_shortcut);
}

// a function that gets called by get_fn_config_and_targets(ctrl, real_fn, shortcut_fn, shortcut_target, __func__);
void get_fn_config_and_targets(struct fn_ctrl *ctrl, void **real_fn, void **shortcut_fn, void *shortcut_target, const char *fn){
  config_fn(ctrl, fn);
    // Get address of real_write
  *real_fn = dlsym(RTLD_NEXT, fn);

  if(ctrl->do_shortcut){
    *shortcut_fn = (void *)sym_get_fn_address(shortcut_target);

      // printf("shortcut_%s is %p \n", __func__, *shortcut_fn);

      // todo resolve this
      printf("warning: unconditionally elevating\n");
      sym_elevate();
    }

}

// TODO: assert const
void ingress_work(struct fn_ctrl *ctrl){
  // User passed -e write, we will elevate before unconditionally
  if(ctrl->sandwich_fn){
    if(ctrl->enter_elevated){
    sym_elevate();
    }else{
      // User passed -l write, we will lower before unconditionally
      // printf("doing lower on entry path\n");
      sym_lower();
    }
  }
}

void egress_work(struct fn_ctrl *ctrl){
  // User passed -e write, we will lower after unconditionally
  if(ctrl->sandwich_fn){
    if(ctrl->return_elevated){
     sym_elevate();
   }else{
    //  printf("doing lower on return path\n");
     sym_lower();
   }
  }
}
ssize_t write(int fd, const void* buf, size_t len){

  // This is awkward, but it is an effort to minimize the scope of differences btw fns.

  // Pointer to the ctrl struct.
  struct fn_ctrl *ctrl = &write_ctrl;

  // Shortcut fn has same signature as write
  typedef write_t shortcut_fn_t;

  // typedef int (*shortcut_fn_t)(unsigned int fd, const char *buf, size_t count);
  typedef write_t real_fn_t;

  // Pointer to real_write, generic version
  real_fn_t *real_fn = &real_write;

  // Pointer to shortcut_write, generic version
  shortcut_fn_t *shortcut_fn = &ksys_write;

  // String name of shortcut target
  char *shortcut_target = "ksys_write";


  // If real_write is null, get it from dlsym
  if (! (*real_fn) ) {
    // Place to do one time work
    // Configs ctrl struct with envt variables, gets ptrs to real and shortcut fns
    get_fn_config_and_targets(ctrl, (void **)real_fn, (void **)shortcut_fn, shortcut_target, __func__);
  }

  // Pre fn work e.g. elevation
  ingress_work(ctrl);

  int ret;
  // Call real write
  if(ctrl->do_shortcut){
    // Awkward, but we have to deref the ptr to the fn ptr
    ret = (*shortcut_fn)(fd, buf, len);
  }else{
    ret = (*real_fn)(fd, buf, len);
  }

  egress_work(ctrl);

  return ret;
}

ssize_t read(int fd, void* buf, size_t len){

  // This is awkward, but it is an effort to minimize the scope of differences btw fns.

  // Pointer to the ctrl struct.
  struct fn_ctrl *ctrl = &read_ctrl;

  // Shortcut fn has same signature as write
  typedef read_t shortcut_fn_t;

  // typedef int (*shortcut_fn_t)(unsigned int fd, const char *buf, size_t count);
  typedef read_t real_fn_t;

  // Pointer to real_write, generic version
  real_fn_t *real_fn = &real_read;

  // Pointer to shortcut_write, generic version
  shortcut_fn_t *shortcut_fn = &ksys_read;

  // String name of shortcut target
  char *shortcut_target = "ksys_read";


  // If real_write is null, get it from dlsym
  if (! (*real_fn) ) {
    // Place to do one time work
    // Configs ctrl struct with envt variables, gets ptrs to real and shortcut fns
    get_fn_config_and_targets(ctrl, (void **)real_fn, (void **)shortcut_fn, shortcut_target, __func__);
  }

  // Pre fn work e.g. elevation
  ingress_work(ctrl);

  int ret;
  // Call real write
  if(ctrl->do_shortcut){
    // Awkward, but we have to deref the ptr to the fn ptr
    ret = (*shortcut_fn)(fd, buf, len);
  }else{
    ret = (*real_fn)(fd, buf, len);
  }

  egress_work(ctrl);

  return ret;
}