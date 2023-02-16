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
// include for var args
#include <stdarg.h>

// Include for cpu_set_t
#include <sched.h>

// TODO: Maybe move this to makefile -I?

#include <sys/poll.h>

// include for epoll_event
#include <sys/epoll.h>

#include "deep_sc/deep_sc.h"

// Just learned this black magic
extern char **environ;

struct cache_elem* sym_cache;

// Function that prints string in red
void print_red(const char *str) { printf("\033[1;31m%s\033[0m", str); }

// Signal handler for SIGUSR1
// NOTE: We removed the args here int signum, dunno if that's safe
void sigusr1_handler() {
  printf("Received SIGUSR1\n");
  printf("lowering now\n");
  sym_lower();
}
// Signal handler for SIGUSR2
void sigusr2_handler() {
  printf("Received SIGUSR2\n");
  printf("elevating now\n");
  sym_elevate();
}

// Signal handler for SIGSYS
void sigsys_handler() {
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
  // Allocate the sym cache really do this in the lib.
  sym_cache = (struct cache_elem*) calloc(SYM_CACHE_SZ, (sizeof(struct cache_elem)));

  // function that is called when the library is loaded
  print_red("SClib: ");
  printf("Shortcut Lib: for interposing syscalls and shortcutting\n");

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

  // print_red("Done initializing\n");
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
  // printf("elevate_fn: %s\n", buf);
}

// Only works for ksys
void build_shortcut_envt_var(char *buf, char *prefix, const char *fn,
                             char *suffix, char *sc_target) {

  // Copy prefix "SHORTCUT_"
  strcpy(buf, prefix);
  // Copy fn
  strcat(buf, fn);
  // Copy _TO_
  strcat(buf, "_TO_");
  // The kernel target
  strcat(buf, sc_target);
  // Copy suffix
  strcat(buf, suffix);

  // printf("elevate_fn: %s\n", buf);
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

void config_fn_shortcut(struct fn_ctrl *ctrl, const char *fn, char *sc_target) {
  // printf("config shortcut fn is %s\n", fn);

  // Manage lower case if necessary.
  char *shortcut_fn = malloc(strlen("SHORTCUT_") + strlen(fn) + strlen("_TO_") +
                             strlen(sc_target) + strlen("=1") + 1);

  build_shortcut_envt_var(shortcut_fn, "SHORTCUT_", fn, "=1", sc_target);

  if (envt_var_exists(shortcut_fn)) {
    print_red("SClib: ");
    printf("%s: requested shortcut\n", fn);
    // If already sandwiching for elevate, then we can't lower
    assert(ctrl->sandwich_fn == false);

    ctrl->sandwich_fn = false;
    ctrl->enter_elevated = false;
    ctrl->return_elevated = false;
    ctrl->do_shortcut = true;
  } else {
    print_red("SClib: ");
    printf("%s: interposed, not shortcutting\n", fn);
  }

  free(shortcut_fn);
}

// Sets function control flags
void config_fn(struct fn_ctrl *ctrl, const char *fn, char *sc_target) {
  init_fn_ctrl(ctrl);
  // Check for elevate envt variable
  // printf("Checking for elevate envt variable\n");
  // printf("envt var:%s\n", "ELEVATE" fn "=1");

  // Manage elevate case if necessary.
  config_fn_elevate(ctrl, fn);

  // Manage lower case if necessary.
  config_fn_lower(ctrl, fn);

  // Manage shortcut case if necessary.
  config_fn_shortcut(ctrl, fn, sc_target);
}

// Print all fields of wr_ctrl struct
void print_ctrl(struct fn_ctrl *ctrl) {
  printf("ctrl->sandwich_fn: %d\n", ctrl->sandwich_fn);
  printf("ctrl->enter_elevated: %d\n", ctrl->enter_elevated);
  printf("ctrl->return_elevated: %d\n", ctrl->return_elevated);
  printf("ctrl->do_shortcut: %d\n", ctrl->do_shortcut);
}

// a function that gets called by get_fn_config_and_targets(ctrl, real_fn,
// shortcut_fn, sc_target, __func__);
void get_fn_config_and_targets(struct fn_ctrl *ctrl, void **real_fn,
                               void **shortcut_fn, char *sc_target,
                               const char *fn) {
  fprintf(stderr, "get_fn_config_and_targets");
  config_fn(ctrl, fn, sc_target);

  // Get address of real_write
  *real_fn = dlsym(RTLD_NEXT, fn);

  if (ctrl->do_shortcut) {
    *shortcut_fn = (void *)sym_get_fn_address(sc_target);

    // printf("shortcut_%s is %p \n", __func__, *shortcut_fn);

    // todo resolve this
    printf("warning: unconditionally elevating\n");
    sym_elevate();
  }
}

// TODO: assert const
void ingress_work(struct fn_ctrl *ctrl, const char *fn_name) {


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

// NOTE: Our rule for getting these fn targets is:
// 1. Objdump vmlinux and grep for "__x64_sys_" + fn
// 2a. if non zero args, find the fn it calls, and use that as the target
// 2b. if zero args, use "__64_sys_" + fn as the target
/* MAKE_STRUCTS_AND_FN_3(write, "__x64_sys_write", ssize_t, int, fd, const void *, buf, size_t, count) */
/* MAKE_STRUCTS_AND_FN_3(read, "__x64_sys_read", ssize_t, int, fd, void *, buf, size_t, count) */
#if 0

MAKE_STRUCTS_AND_FN_0(getppid, "__x64_sys_getppid", pid_t)
MAKE_STRUCTS_AND_FN_0(getpid, "__x64_sys_getpid", pid_t)

// mmap
MAKE_STRUCTS_AND_FN_6(mmap, "__x64_sys_mmap", void *, void *, addr, size_t, len, int, prot, int, flags, int, fd, off_t, offset)

// // munmap
// #error // target is wrong
MAKE_STRUCTS_AND_FN_2(munmap, "__x64_sys_munmap", int, void *, addr, size_t, len)

// // recvfrom
// #error // target is wrong
// MAKE_STRUCTS_AND_FN_6(recvfrom, "ksys_recvfrom", ssize_t, int, fd, void *, buf, size_t, len, unsigned int, flags, struct sockaddr *, src_addr, socklen_t *, addrlen)
// // sendto
// #error // target is wrong
// MAKE_STRUCTS_AND_FN_6(sendto, "ksys_sendto", ssize_t, int, fd, const void *, buf, size_t, len, unsigned int, flags, const struct sockaddr *, dest_addr, socklen_t, addrlen)
// // poll
// #error // target is wrong
MAKE_STRUCTS_AND_FN_3(poll, "__x64_sys_poll", int, struct pollfd *, fds, nfds_t, nfds, int, timeout)
// // select
// #error // target is wrong
MAKE_STRUCTS_AND_FN_5(select, "__x64_sys_select", int, int, nfds, fd_set *, readfds, fd_set *, writefds, fd_set *, exceptfds, struct timeval *, timeout)

MAKE_STRUCTS_AND_FN_4(epoll_wait, "__x64_sys_epoll_wait", int, int, epfd, struct epoll_event *, events, int, maxevents, int, timeout)

MAKE_STRUCTS_AND_FN_0(fork, "__x64_sys_fork", pid_t)

// fork XXX don't know if that's the right target, only need it to 
// lower for now
// MAKE_STRUCTS_AND_FN_0(fork, "__do_sys_fork", pid_t)

// A function called syscall which takes a syscall number and a variable number
// of arguments. It will call the real syscall with the same arguments.
// This will set a bit in the ctrl struct to indicate that we are coming from
// this entry path as opposed to a glibc call like read().

long (*real_syscall)(long, ...) = NULL;

long syscall_entry_ctr = 0;
// NOTE: TODO, we don't send the call straight to syscall()
// it's either going to the glibc fn (like read()) or the ksys fn (like
// ksys_read) This could be made more symmetric with some modification to the
// macro.
long syscall(long syscall_vec, ...) {

    if (real_syscall == NULL) {
        real_syscall = dlsym(RTLD_NEXT, "syscall");
    }

    va_list args;
    va_start(args, syscall_vec);

    long ret = -1;
    switch (syscall_vec) {
    case SYS_getpid:
        return getpid();
        break;
    case SYS_getppid:
        return getppid();
        break;
    case SYS_mmap: {
        void *addr = va_arg(args, void *);
        size_t length = va_arg(args, size_t);
        int prot = va_arg(args, int);
        int flags = va_arg(args, int);
        int fd = va_arg(args, int);
        off_t offset = va_arg(args, off_t);
        return (long)mmap(addr, length, prot, flags, fd, offset);
    } break;
    case SYS_munmap: {
        void *addr = va_arg(args, void *);
        size_t length = va_arg(args, size_t);
        return munmap(addr, length);
    } break;
    // case SYS_sendto: {
    //     int fd = va_arg(args, int);
    //     void *buf = va_arg(args, void *);
    //     size_t count = va_arg(args, size_t);
    //     int flags = va_arg(args, int);
    //     struct sockaddr *dest_addr = va_arg(args, struct sockaddr *);
    //     socklen_t addrlen = va_arg(args, socklen_t);
    //     return real_syscall(SYS_sendto, fd, buf, count, flags, dest_addr,
    //                         addrlen);
    // } break;
    // case SYS_recvfrom: {
    //     int fd = va_arg(args, int);
    //     void *buf = va_arg(args, void *);
    //     size_t count = va_arg(args, size_t);
    //     int flags = va_arg(args, int);
    //     struct sockaddr *src_addr = va_arg(args, struct sockaddr *);
    //     socklen_t *addrlen = va_arg(args, socklen_t *);
    //     return real_syscall(SYS_recvfrom, fd, buf, count, flags, src_addr,
    //                         addrlen);
    // } break;
    case SYS_select: {
        int nfds = va_arg(args, int);
        fd_set *readfds = va_arg(args, fd_set *);
        fd_set *writefds = va_arg(args, fd_set *);
        fd_set *exceptfds = va_arg(args, fd_set *);
        struct timeval *timeout = va_arg(args, struct timeval *);
        return select(nfds, readfds, writefds, exceptfds, timeout);
    } break;
    case SYS_poll: {
        struct pollfd *fds = va_arg(args, struct pollfd *);
        size_t nfds = va_arg(args, size_t);
        int timeout = va_arg(args, int);
        return poll(fds, nfds, timeout);
    } break;
    case SYS_write: {
        int fd = va_arg(args, int);
        void *buf = va_arg(args, void *);
        size_t count = va_arg(args, size_t);
        return write(fd, buf, count);
    } break;
    case SYS_read: {
        int fd = va_arg(args, int);
        void *buf = va_arg(args, void *);
        size_t count = va_arg(args, size_t);
        return read(fd, buf, count);
    } break;
    default: {
        uint64_t arg1, arg2, arg3, arg4, arg5, arg6;
        arg1 = va_arg(args, uint64_t);
        arg2 = va_arg(args, uint64_t);
        arg3 = va_arg(args, uint64_t);
        arg4 = va_arg(args, uint64_t);
        arg5 = va_arg(args, uint64_t);
        arg6 = va_arg(args, uint64_t);
        // printf("arg1: %ld arg2: %ld arg3: %ld arg4: %ld arg5: %ld arg6:
        // %ld\n", arg1, arg2, arg3, arg4, arg5, arg6);
        return real_syscall(syscall_vec, arg1, arg2, arg3, arg4, arg5, arg6);
    }
    }
    va_end(args);

    // No one should get here.
    assert(false);
    return ret;
}

#endif

// Type for close function
typedef int (*close_t)(int fd);
int close(int fd) {
  // invalidate fdth element of sym_cache array
  /* printf("close called on fd %d\n", fd); */
  invalidate_cache_elem(fd);
  close_t my_close = (close_t) dlsym(RTLD_NEXT, "close");
  return my_close(fd);
}

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
uint64_t write_target = 0;
write_t real_write = ((void *)0);
struct fn_ctrl write_ctrl = {0, 0, 0, 0};
my_tcp_sendmsg_t tcp_sendmsg = ((void *)0);


ssize_t write(int fd, const void *buf, size_t count) {
  /* fprintf(stderr, "made it to write\n"); */
  if (!real_write) {
    get_fn_config_and_targets(&write_ctrl, (void **)&real_write,
                              (void **)&tcp_sendmsg, "tcp_sendmsg", __func__);
  }

  // Print caller return address 2 above
  uint64_t addr = (uint64_t) __builtin_extract_return_addr (__builtin_return_address (0));
  /* fprintf(stderr, "caller return address is %lx\n", addr); */

  ingress_work(&write_ctrl, __func__);
  int ret;

  if (do_sc(write_ctrl.do_shortcut) && ( addr == 0x5007b1) ) {
    // Checked here bc this runs before write.
    // TODO: Put this in write_populate_cache?
    assert(fd < SYM_CACHE_SZ);

    // TODO: set tcp_sendmsg if it's null
    if(write_target == 0){
      write_target = addr;
    } else {
      // lemme know if it changes.
      assert(write_target == addr);
    }

    // if the cache element is populated
    if (sym_cache[fd].send.valid == true) {
      /* fprintf(stderr, "Fast write\n"); */
      ret = cached_tcp_sendmsg_path(fd, buf, count);
      // ret = real_write(fd, buf, count);
    } else {
      /* printf("populate cache normal write path\n"); */
      ret = write_populate_cache(fd, buf, count);
      // ret = real_write(fd, buf, count);
    }

  } else {
    /* printf("normal write path\n"); */
    ret = real_write(fd, buf, count);
  }
  egress_work(&write_ctrl);
  return ret;
}
// TODO: Invalidate cache on close.

uint64_t read_target = 0;
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
read_t real_read = ((void *)0);
struct fn_ctrl read_ctrl = {0, 0, 0, 0};
my_tcp_recvmsg_t tcp_recvmsg = ((void *)0);

ssize_t read(int fd, void *buf, size_t count) {
  if (!real_read) {
    get_fn_config_and_targets(&read_ctrl, (void **)&real_read,
                              (void **)&tcp_recvmsg, "tcp_recvmsg", __func__);
  }
  // Print caller return address 2 above
  uint64_t addr = (uint64_t) __builtin_extract_return_addr (__builtin_return_address (0));
  /* fprintf(stderr, "caller return address is %lx\n", addr); */

  ingress_work(&read_ctrl, __func__);
  int ret;
  //TODO: fix this address
  if (do_sc(read_ctrl.do_shortcut && (addr == 0x500751))) {
    if(read_target == 0){
      read_target = addr;
    } else {
      // lemme know if it changes.
      assert(read_target == addr);
    }
    if (sym_cache[fd].recv.valid == true) {
      /* fprintf(stderr, "Fast read\n"); */
      ret = cached_tcp_recvmsg_path(fd, buf, count);
    } else {
      /* fprintf(stderr, "populate cache normal read path\n"); */
      ret = read_populate_cache(fd, buf, count);
    }
  } else {
      /* fprintf(stderr, "normal read path\n"); */
      ret = real_read(fd, buf, count);
  }
  egress_work(&read_ctrl);
  return ret;
}
