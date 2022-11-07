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
  bool enter_elevated;
  bool return_elevated;
  bool do_shortcut;
};

// Just learned this black magic
extern char** environ;

// #define BUFSIZE 4096

// typedef void (*void_fn_ptr)(unsigned long);
// typedef int (*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
// typedef int (*ksys_read_t)(unsigned int fd, const char *buf, size_t count);
// ksys_write_t ksys_wr_sc;
// ksys_read_t ksys_rd_sc;


// Function that prints string in red
void print_red(const char* str){
    printf("\033[1;31m%s\033[0m", str);
}

// This is dumb, but we need something quick.
// We've used these to elevate

// We've used these to lower

// We've used these to shortcut
struct fn_ctrl rd_ctrl; //wr_ctrl, getpid, getppid;


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


// Use envt vars to process function controls
void process_fn_ctrls(){
  printf("process_fn_ctrls\n\n");

  // Elevate and lower NYI

  // Find functions to shortcut.
  for (size_t i = 0; environ[i] != NULL; i++) {
    if (strncmp(environ[i], "SHORTCUT", 8) == 0) {
      // Compare environment variable to known strings
      char *wr_str = "SHORTCUT_write_TO_ksys_write=1";
      char *rd_str = "SHORTCUT_read_TO_ksys_read=1";
      if(strncmp(environ[i], wr_str, strlen(wr_str)) == 0){
        printf("Need to shortcut write to ksys_write\n");
      } else if (strncmp(environ[i], rd_str, strlen(rd_str)) == 0) {
        printf("Need to shortcut rd to ksys_rd\n");
      } else{
        // Print error with unexpected shortcut and exit
        printf("Unexpected shortcut: %s\n", environ[i]);
        exit(1);
      }
      
    }
  }


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
  process_fn_ctrls();  // Initialize the function control structs

  // If environment variable 'BEGIN_ELE=1' is set, elevate
  if (envt_var_exists("BEGIN_ELE=1")) {
    printf("Elevating\n");
  //  elevate_fn(&rd_ctrl);
  }



  print_red("Done initializing\n");

}

void __attribute__ ((destructor)) cleanUp(void) {
  // function that is called when the library is »closed«.
  sym_lower();
}

// Our interposer
// Function pointer typedef for read glibc call
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
// Function pointer typedef for write glibc call
typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);

read_t real_read = NULL;
write_t real_write = NULL;
ssize_t read(int fd, void* buf, size_t len){
   if (!real_read) {
    real_read = dlsym(RTLD_NEXT, "read");
    printf("real_read is %p \n", real_read);
  }

  // Call real read
  return real_read(fd, buf, len);
}

ssize_t write(int fd, const void* buf, size_t len){
  // If real_write is null, get it from dlsym
  if (!real_write) {
    real_write = dlsym(RTLD_NEXT, "write");
    printf("real_write is %p \n", real_write);
  }
  // Call real write
  return real_write(fd, buf, len);

}
