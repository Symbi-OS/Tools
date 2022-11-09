  #define _GNU_SOURCE

#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../../Symlib/include/LINF/sym_all.h"
// Include for bool
#include <stdbool.h>

#include <stdio.h>
#include <dlfcn.h>

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

// Just learned this black magic
extern char** environ;

// Function that prints string in red
void print_red(const char* str){
    printf("\033[1;31m%s\033[0m", str);
}

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

// Function that initializes a fn_ctrl struct
void init_fn_ctrl(struct fn_ctrl* ctrl){
  ctrl->sandwich_fn = false;
  ctrl->enter_elevated = false;
  ctrl->return_elevated = false;
  ctrl->do_shortcut = false;
}

// Elevate fn
void elevate_fn(struct fn_ctrl* ctrl){
}
void lower_fn(struct fn_ctrl* ctrl){
}
void shortcut_fn(struct fn_ctrl* ctrl){
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
  print_sc_envt_vars();

  // Initialize function controls
  // process_fn_ctrls();  // Initialize the function control structs

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
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);

// Where the calls would have gone to before us.
read_t real_read = NULL;
write_t real_write = NULL;

// Are we doing elevating or lowering on this call?
// Can't do both elevating and lowering
// If shortcutting, need to either be elevated from the beginning or elevate on the fly
// If shortcutting, lower can't be used.

struct fn_ctrl wr_ctrl;

ssize_t read(int fd, void* buf, size_t len){
  if (!real_read) {
    // Place to do one time work.
    real_read = dlsym(RTLD_NEXT, "read");
    printf("real_read is %p \n", real_read);
  }

  // Call real read
  return real_read(fd, buf, len);
}
// Takes buffer, prefix, fn, and suffix returns a string.
  void build_envt_var(char * buf, char * prefix, const char * fn, char * suffix){

    // Copy prefix
    strcpy(buf, prefix);
    // Copy fn
    strcat(buf, fn);
    // Copy suffix
    strcat(buf, suffix);
  printf("elevate_fn: %s\n", buf);


  }

// Sets elevate and lower flags for function
void config_fn(struct fn_ctrl *wr_ctrl, const char *fn){
  init_fn_ctrl(wr_ctrl);
  // Check for elevate envt variable
  printf("Checking for elevate envt variable\n");
  // printf("envt var:%s\n", "ELEVATE" fn "=1");

  char *elevate_fn = malloc(strlen("ELEVATE_") + strlen(fn) + strlen("=1") + 1);
  // check if malloc failed
  if (elevate_fn == NULL) {
    printf("Malloc failed\n");
    exit(1);
  }

  // Builds elevate_fn
  build_envt_var(elevate_fn, "ELEVATE_", fn, "=1");

  if(envt_var_exists(elevate_fn)){
    printf("Found elevate envt variable\n");
    wr_ctrl->sandwich_fn = true;
    wr_ctrl->enter_elevated = true;
    wr_ctrl->return_elevated = false;
  }else{
    printf("no envt variable\n");
  }
  free(elevate_fn);

  char *lower_fn = malloc(strlen("LOWER_") + strlen(fn) + strlen("=1") + 1);
  build_envt_var(lower_fn, "LOWER_", fn, "=1");
  if(envt_var_exists(lower_fn)){
    // If already sandwiching for elevate, then we can't lower
    assert(wr_ctrl->sandwich_fn == false);

    printf("Found lower envt variable\n");
    wr_ctrl->sandwich_fn = true;
    wr_ctrl->enter_elevated = false;
    wr_ctrl->return_elevated = true;
  }else{
    printf("no envt variable\n");
  }
  free(lower_fn);

  if(envt_var_exists("SHORTCUT_write_TO_ksys_write=1")){
    // We've got a problem if elevate and lower are set
    assert(wr_ctrl->sandwich_fn == false);

    printf("Found shortcut envt variable\n");
    wr_ctrl->sandwich_fn = false;
    wr_ctrl->do_shortcut = true;

  }
}

// Print all fields of wr_ctrl struct
void print_wr_ctrl(struct fn_ctrl *wr_ctrl){
  printf("wr_ctrl->sandwich_fn: %d\n", wr_ctrl->sandwich_fn);
  printf("wr_ctrl->enter_elevated: %d\n", wr_ctrl->enter_elevated);
  printf("wr_ctrl->return_elevated: %d\n", wr_ctrl->return_elevated);
  printf("wr_ctrl->do_shortcut: %d\n", wr_ctrl->do_shortcut);
}

typedef int (*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
ksys_write_t ksys_write = NULL;

ssize_t write(int fd, const void* buf, size_t len){
  // If real_write is null, get it from dlsym
  if (!real_write) {

    // Place to do one time work
    config_fn(&wr_ctrl, __func__);

    // print the name of this function __func__
    real_write = dlsym(RTLD_NEXT, "write");
    printf("real_write is %p \n", real_write);

    if(wr_ctrl.do_shortcut){
      ksys_write = (ksys_write_t)sym_get_fn_address("ksys_write");
      // todo resolve this
      printf("warning: unconditionally elevating\n");
      sym_elevate();
    }
  }
  // print_wr_ctrl(&wr_ctrl);

  // User passed -e write, we will elevate before unconditionally
  if(wr_ctrl.sandwich_fn){
    if(wr_ctrl.enter_elevated){
    sym_elevate();
    }else{
      // User passed -l write, we will lower before unconditionally
      // printf("doing lower on entry path\n");
      sym_lower();
    }
  }
  int ret;
  // Call real write
  // todo: case for shortcut
  if(wr_ctrl.do_shortcut){
    // printf("doing shortcut\n");
    ret = ksys_write(fd, buf, len);
  }else{

    ret = real_write(fd, buf, len);

  }

  // User passed -e write, we will lower after unconditionally
  if(wr_ctrl.sandwich_fn){
    if(wr_ctrl.return_elevated){
     sym_elevate();
   }else{
    //  printf("doing lower on return path\n");
     sym_lower();
   }
  }

  return ret;
}
