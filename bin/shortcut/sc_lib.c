#define _GNU_SOURCE

#include <dlfcn.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "sc_lib.h"
#ifdef DEEP_SHORTCUT
#include "deep_sc/deep_sc.h"
#endif

extern char **environ;

struct cache_elem *sym_cache;

// Function that prints string in red
void print_red(const char *str) { fprintf(stderr, "\033[1;31m%s\033[0m", str); }

// Signal handler for SIGUSR1
// NOTE: We removed the args here int signum, dunno if that's safe
void sigusr1_handler() {
    fprintf(stderr, "Received SIGUSR1\n");
    fprintf(stderr, "lowering now\n");
    sym_lower();
}
// Signal handler for SIGUSR2
void sigusr2_handler() {
    fprintf(stderr, "Received SIGUSR2\n");
    fprintf(stderr, "elevating now\n");
    sym_elevate();
}

// Signal handler for SIGSYS
void sigsys_handler() {
    fprintf(stderr, "Received SIGSYS\n");

    // Print if shortcut is on or off
    if (sc_disable) {
        fprintf(stderr, "Shortcut is on, turning off\n");
    } else {
        fprintf(stderr, "Shortcut is off, turning on\n");
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
#ifdef DEEP_SHORTCUT
    sym_cache =
        (struct cache_elem *)calloc(SYM_CACHE_SZ, (sizeof(struct cache_elem)));
#endif
    // function that is called when the library is loaded
    print_red("SClib: ");
    fprintf(stderr,
            "Shortcut Lib: for interposing syscalls and shortcutting\n");

    // for debugging
    // print_sc_envt_vars();

    // Register signal handlers
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGSYS, sigsys_handler);

    // If environment variable 'BEGIN_ELE=1' is set, elevate
    if (envt_var_exists("BEGIN_ELE=1")) {
        fprintf(stderr, "Elevating\n");
        sym_elevate();
    }

    // print_red("Done initializing\n");
}

void __attribute__((destructor)) cleanUp(void) {
    // function that is called when the library is »closed«.
#ifdef DEEP_SHORTCUT
    free(sym_cache);
#endif
    sym_lower();
    printf("Normal path count is %d\n", normal_path_ct);
    printf("SC path count is %d\n", sc_path_ct);
}

// Takes buffer, prefix, fn, and suffix
void build_envt_var(char *buf, char *prefix, const char *fn, char *suffix) {
    // Copy prefix
    strcpy(buf, prefix);
    // Copy fn
    strcat(buf, fn);
    // Copy suffix
    strcat(buf, suffix);
    // fprintf(stderr, "elevate_fn: %s\n", buf);
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

    // fprintf(stderr, "elevate_fn: %s\n", buf);
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
    char *elevate_fn =
        malloc(strlen("ELEVATE_") + strlen(fn) + strlen("=1") + 1);
    // check if malloc failed
    if (elevate_fn == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(1);
    }

    // Builds elevate_fn string "ELEVATE_fn=1"
    build_envt_var(elevate_fn, "ELEVATE_", fn, "=1");

    // If environment variable 'ELEVATE_fn=1' is set, config fn_ctrl for elevate
    if (envt_var_exists(elevate_fn)) {
        fprintf(stderr, "Do config for elevate\n");
        config_fn_ctrl_for_elevate(ctrl);
    } else {
        // fprintf(stderr, "no envt variable\n");
    }

    free(elevate_fn);
}

void config_fn_lower(struct fn_ctrl *ctrl, const char *fn) {

    // Manage lower case if necessary.
    char *lower_fn = malloc(strlen("LOWER_") + strlen(fn) + strlen("=1") + 1);
    // Check that malloc didn't fail
    if (lower_fn == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(1);
    }
    build_envt_var(lower_fn, "LOWER_", fn, "=1");
    if (envt_var_exists(lower_fn)) {
        // If already sandwiching for elevate, then we can't lower
        assert(ctrl->sandwich_fn == false);

        fprintf(stderr, "Found lower envt variable\n");
        config_fn_ctrl_for_lower(ctrl);
    } else {
        // fprintf(stderr, "no envt variable\n");
    }
    free(lower_fn);
}

void config_fn_shortcut(struct fn_ctrl *ctrl, const char *fn, char *sc_target) {
    // fprintf(stderr, "config shortcut fn is %s\n", fn);

    // Manage lower case if necessary.
    char *shortcut_fn =
        malloc(strlen("SHORTCUT_") + strlen(fn) + strlen("_TO_") +
               strlen(sc_target) + strlen("=1") + 1);
    // Check that malloc didn't fail
    if (shortcut_fn == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(1);
    }

    build_shortcut_envt_var(shortcut_fn, "SHORTCUT_", fn, "=1", sc_target);

    if (envt_var_exists(shortcut_fn)) {
        print_red("SClib: ");
        fprintf(stderr, "[%s]: requested shortcut\n", fn);
        // If already sandwiching for elevate, then we can't lower
        assert(ctrl->sandwich_fn == false);

        ctrl->sandwich_fn = false;
        ctrl->enter_elevated = false;
        ctrl->return_elevated = false;
        ctrl->do_shortcut = true;
    } else {
        print_red("SClib: ");
        fprintf(stderr, "[%s]: interposed, not shortcutting\n", fn);
    }

    free(shortcut_fn);
}

// Sets function control flags
void config_fn(struct fn_ctrl *ctrl, const char *fn, char *sc_target) {
    init_fn_ctrl(ctrl);
    // Check for elevate envt variable
    // fprintf(stderr, "Checking for elevate envt variable\n");
    // fprintf(stderr, "envt var:%s\n", "ELEVATE" fn "=1");

    // Manage elevate case if necessary.
    config_fn_elevate(ctrl, fn);

    // Manage lower case if necessary.
    config_fn_lower(ctrl, fn);

    // Manage shortcut case if necessary.
    config_fn_shortcut(ctrl, fn, sc_target);
}

// Print all fields of wr_ctrl struct
void print_ctrl(struct fn_ctrl *ctrl) {
    fprintf(stderr, "ctrl->sandwich_fn: %d\n", ctrl->sandwich_fn);
    fprintf(stderr, "ctrl->enter_elevated: %d\n", ctrl->enter_elevated);
    fprintf(stderr, "ctrl->return_elevated: %d\n", ctrl->return_elevated);
    fprintf(stderr, "ctrl->do_shortcut: %d\n", ctrl->do_shortcut);
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

        // fprintf(stderr, "shortcut_%s is %p \n", __func__, *shortcut_fn);

        // todo resolve this
        fprintf(stderr, "warning: unconditionally elevating\n");
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
            // fprintf(stderr, "doing lower on entry path\n");
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
            //  fprintf(stderr, "doing lower on return path\n");
            sym_lower();
        }
    }
}

bool do_sc(bool do_sc_for_fn) {
    // Master control, if this isn't set, no chance we're shortcutting.
    if (!do_sc_for_fn)
        return false;

    // Shortcutting is enabled, we have checks to do:

    // We allow for a global shortcut disable. This is useful for interactive
    // mode.
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

#ifndef DEEP_SHORTCUT

// ssize_t pread(int fd, void *buf, size_t count, off_t offset);

// ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

// MAKE_STRUCTS_AND_FN_4(__libc_pwrite, "__x64_sys_pwrite64", ssize_t, int, fd, const void *,
//                       buf, size_t, nbytes, off_t, offset)

MAKE_STRUCTS_AND_FN_4(pwrite64, "__x64_sys_pwrite64", ssize_t, int, fd, const void *,
                      buf, size_t, count, off_t, offset)
// MAKE_STRUCTS_AND_FN_4(pwritev, "__x64_sys_pwritev", ssize_t, int, fd, const struct iovec *,
//                       iov, int, iovcnt, off_t, offset)
// pwritev64
// pwritev64v2
// MAKE_STRUCTS_AND_FN_4(pread, "__x64_sys_pread64", ssize_t, int, fd, void *,
//                       buf, size_t, count, off_t, offset);

// ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);

MAKE_STRUCTS_AND_FN_3(sendmsg, "__x64_sys_sendmsg", ssize_t, int, sockfd, const struct msghdr*,
                      msg, int, flags)

MAKE_STRUCTS_AND_FN_3(write, "__x64_sys_write", ssize_t, int, fd, const void *,
                      buf, size_t, count)
MAKE_STRUCTS_AND_FN_3(read, "__x64_sys_read", ssize_t, int, fd, void *, buf,
                      size_t, count)
MAKE_STRUCTS_AND_FN_0(getppid, "__x64_sys_getppid", pid_t)
MAKE_STRUCTS_AND_FN_0(getpid, "__x64_sys_getpid", pid_t)
MAKE_STRUCTS_AND_FN_6(mmap, "__x64_sys_mmap", void *, void *, addr, size_t, len,
                      int, prot, int, flags, int, fd, off_t, offset)
MAKE_STRUCTS_AND_FN_2(munmap, "__x64_sys_munmap", int, void *, addr, size_t,
                      len)
MAKE_STRUCTS_AND_FN_3(poll, "__x64_sys_poll", int, struct pollfd *, fds, nfds_t,
                      nfds, int, timeout)
MAKE_STRUCTS_AND_FN_5(select, "__x64_sys_select", int, int, nfds, fd_set *,
                      readfds, fd_set *, writefds, fd_set *, exceptfds,
                      struct timeval *, timeout)
MAKE_STRUCTS_AND_FN_4(epoll_wait, "__x64_sys_epoll_wait", int, int, epfd,
                      struct epoll_event *, events, int, maxevents, int,
                      timeout)
MAKE_STRUCTS_AND_FN_0(fork, "__x64_sys_fork", pid_t)


// fork XXX don't know if that's the right target, only need it to
// lower for now
// MAKE_STRUCTS_AND_FN_0(fork, "__do_sys_fork", pid_t)

// A function called syscall which takes a syscall number and a variable number
// of arguments. It will call the real syscall with the same arguments.
// This will set a bit in the ctrl struct to indicate that we are coming from
// this entry path as opposed to a glibc call like read().
#if 0
long (*real_syscall)(long, ...) = NULL;

long syscall_entry_ctr = 0;
// NOTE: TODO, we don't send the call straight to syscall()
// it's either going to the glibc fn (like read()) or the ksys fn (like
// ksys_read) This could be made more symmetric with some modification to the
// macro.
long syscall(long syscall_vec, ...) {
    struct pt_regs regs;
    // Get the damn args
    asm volatile("movq %%rdi, %0" : "=m"(regs.rdi) : : "memory");
    asm volatile("movq %%rsi, %0" : "=m"(regs.rsi) : : "memory");
    asm volatile("movq %%rdx, %0" : "=m"(regs.rdx) : : "memory");
    asm volatile("movq %%rcx, %0" : "=m"(regs.rcx) : : "memory");
    asm volatile("movq %%r8, %0" : "=m"(regs.r8) : : "memory");
    asm volatile("movq %%r9, %0" : "=m"(regs.r9) : : "memory");

    if (real_syscall == NULL) {
        real_syscall = dlsym(RTLD_NEXT, "syscall");
    }

    // va_list args;
    // va_start(args, syscall_vec);

    long ret = -1;
    switch (syscall_vec) {
    case SYS_getpid:
        return getpid();
        break;
    case SYS_getppid:
        fprintf(stderr, "call glibc getppid\n");
        return getppid();
        break;
    case SYS_mmap: {
        fprintf(stderr, "mmap nyi\n");
        exit(1);
        // void *addr = va_arg(args, void *);
        // size_t length = va_arg(args, size_t);
        // int prot = va_arg(args, int);
        // int flags = va_arg(args, int);
        // int fd = va_arg(args, int);
        // off_t offset = va_arg(args, off_t);
        // return (long)mmap(addr, length, prot, flags, fd, offset);
    } break;
    case SYS_munmap: {
        fprintf(stderr, "munmap nyi\n");
        exit(1);
        return 0;
        // void *addr = va_arg(args, void *);
        // size_t length = va_arg(args, size_t);
        // return munmap(addr, length);
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
        // int nfds = va_arg(args, int);
        // fd_set *readfds = va_arg(args, fd_set *);
        // fd_set *writefds = va_arg(args, fd_set *);
        // fd_set *exceptfds = va_arg(args, fd_set *);
        // struct timeval *timeout = va_arg(args, struct timeval *);
        // return select(nfds, readfds, writefds, exceptfds, timeout);
        fprintf(stderr, "select nyi\n");
        exit(-1);
    } break;
    case SYS_poll: {
        // struct pollfd *fds = va_arg(args, struct pollfd *);
        // size_t nfds = va_arg(args, size_t);
        // int timeout = va_arg(args, int);
        // return poll(fds, nfds, timeout);
        fprintf(stderr, "poll nyi\n");
    } break;
    case SYS_write: {
        // int fd = va_arg(args, int);
        // void *buf = va_arg(args, void *);
        // size_t count = va_arg(args, size_t);
        return write((int)regs.rdi, (char*)regs.rsi, (size_t)regs.rdx);
    } break;
    case SYS_read: {
        // int fd = va_arg(args, int);
        // void *buf = va_arg(args, void *);
        // size_t count = va_arg(args, size_t);
        return read((int)regs.rdi, (char *)regs.rsi, (size_t)regs.rdx);
    } break;
    default: {
        // uint64_t arg1, arg2, arg3, arg4, arg5, arg6;
        // arg1 = va_arg(args, uint64_t);
        // arg2 = va_arg(args, uint64_t);
        // arg3 = va_arg(args, uint64_t);
        // arg4 = va_arg(args, uint64_t);
        // arg5 = va_arg(args, uint64_t);
        // arg6 = va_arg(args, uint64_t);
        // fprintf(stderr, "arg1: %ld arg2: %ld arg3: %ld arg4: %ld arg5: %ld arg6:
        // %ld\n", arg1, arg2, arg3, arg4, arg5, arg6);
        return real_syscall(syscall_vec, regs.rdi, regs.rsi, regs.rdx, regs.rcx, regs.r8, regs.r9);
    }
    }
    // va_end(args);

    // No one should get here.
    assert(false);
    return ret;
}
#endif
#else

// Type for close function
typedef int (*close_t)(int fd);
int close(int fd) {
    // invalidate fdth element of sym_cache array
    /* fprintf(stderr, "close called on fd %d\n", fd); */
    invalidate_cache_elem(fd);
    close_t my_close = (close_t)dlsym(RTLD_NEXT, "close");
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
                                  (void **)&tcp_sendmsg, "tcp_sendmsg",
                                  __func__);
    }

    // Print caller return address 2 above
    uint64_t addr =
        (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0));
    /* fprintf(stderr, "caller return address is %lx\n", addr); */

    ingress_work(&write_ctrl);
    int ret;

    if (do_sc(write_ctrl.do_shortcut) && (addr == 0x5007b1)) {
        // Checked here bc this runs before write.
        // TODO: Put this in write_populate_cache?
        assert(fd < SYM_CACHE_SZ);

        // TODO: set tcp_sendmsg if it's null
        if (write_target == 0) {
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
            /* fprintf(stderr, "populate cache normal write path\n"); */
            ret = write_populate_cache(fd, buf, count);
            // ret = real_write(fd, buf, count);
        }

    } else {
        /* fprintf(stderr, "normal write path\n"); */
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
                                  (void **)&tcp_recvmsg, "tcp_recvmsg",
                                  __func__);
    }
    // Print caller return address 2 above
    uint64_t addr =
        (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0));
    /* fprintf(stderr, "caller return address is %lx\n", addr); */

    ingress_work(&read_ctrl);
    int ret;
    // TODO: fix this address
    if (do_sc(read_ctrl.do_shortcut && (addr == 0x500751))) {
        if (read_target == 0) {
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
#endif
