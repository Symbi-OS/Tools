// Header guard here
#ifndef SC_LIB_H
#define SC_LIB_H
// Include for ssize_t
#include "../../../Symlib/include/LINF/sym_all.h"
#include <stdbool.h>
#include <sys/types.h>

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
typedef long (*entry_t)(struct pt_regs *reg);

// TODO, sig is misleading because it's not a signature, it's a type.
#define MAKE_STRUCTS(fn_name, sig)                                             \
    typedef sig;                                                               \
    fn_name##_t real_##fn_name = NULL;                                         \
    struct fn_ctrl fn_name##_ctrl = {0, 0, 0, 0};                              \
    entry_t sc_target_##fn_name = NULL;

// Make function signatures, needs types and args
#define MAKE_SIG(ret_t, fn_name, ...) ret_t fn_name(__VA_ARGS__)
// Make function call, needs args
#define MAKE_FN_CALL(fn_name, ...) fn_name(__VA_ARGS__)

// NOTE: On x86_64, we believe all syscalls vector to __x64_sys_<syscall_name>.
// When shortcutting, we target these functions because:
// 1. It's consistent (as opposed to going to ksys_read for one and do_sys...
// for another)
// 2. In general, there's non trivial work done between the __x64 entry point
// and the next function.
//    such as generating a new fn signature, or error checking.
// We are assuming here that all x64_sys entrypoints take a pt_regs struct as
// their only argument. We assume the handlers ONLY use the arguemts they need
// and nothing else We populate a superset of the arguments here and assume the
// rest are ignored.

// TODO: Make GCC build this with optimization level 0

void print_args(struct pt_regs *regs) {
    printf("arg0: %lx\n", regs->rdi);
    printf("arg1: %lx\n", regs->rsi);
    printf("arg2: %lx\n", regs->rdx);
    printf("arg3: %lx\n", regs->rcx);
    printf("arg4: %lx\n", regs->r8);
    printf("arg5: %lx\n", regs->r9);
}

// asm volatile("movq %%rdi, %0" : "=m" (regs.rdi));
// asm volatile("movq %%rsi, %0" : "=m" (regs.rsi));
// asm volatile("movq %%rdx, %0" : "=m" (regs.rdx));
// asm volatile("movq %%rcx, %0" : "=m" (regs.rcx))
// asm volatile("movq %%r8, %0" : "=m" (regs.r8));
// asm volatile("movq %%r9, %0" : "=m" (regs.r9));

// Todo: deal with sys vec num and floating arg


// This saves syscall regs in the format that the kernel expects.
// The regs struct will be the only thing passed into the syscall.
// This assumes the syscall only needs these regs, not any others.
// NOTE: RCX goes to r10 bc it's clobbered in syscall instruction, this is
// what syscall() does.
#define STORE_REGS()                                                       \
    asm volatile("movq %%rdi, %0" : "=m"(regs.rdi) : : "memory");          \
    asm volatile("movq %%rsi, %0" : "=m"(regs.rsi) : : "memory");          \
    asm volatile("movq %%rdx, %0" : "=m"(regs.rdx) : : "memory");          \
    asm volatile("movq %%rcx, %0" : "=m"(regs.r10) : : "memory");          \
    asm volatile("movq %%r8, %0" : "=m"(regs.r8) : : "memory");            \
    asm volatile("movq %%r9, %0" : "=m"(regs.r9) : : "memory");

// This deals with first time initialization.
// It may be possible to eliminate this?
#define INITIALIZE_FUNCTION(fn_name, sc_target)                                \
        if (!real_##fn_name) {                                                 \
            get_fn_config_and_targets(                                         \
                &fn_name##_ctrl, (void **)&real_##fn_name,                     \
                (void **)&sc_target_##fn_name, sc_target, __func__);           \
        }
// Save the user stack and switch onto the kernel stack.
// I hate this hardcoded use of getting the kernel stack, do better.
#define HANDLE_SHORTCUT(fn_name, sc_target, ret_t)                             \
    uint64_t user_stack;                                                       \
    asm volatile("mov %%rsp, %0" : "=m"(user_stack) : : "memory");             \
    asm volatile("mov %gs:0x17b90, %rsp");                                     \
    ret = (ret_t)MAKE_FN_CALL(sc_target_##fn_name, &regs);                     \
    asm volatile("mov %0, %%rsp" : : "r"(user_stack));
#if 0
#define CALL_FUNCTION(fn_name, sc_target, ret_t, args) \
    ret_t ret;                                         \
    if (do_sc(fn_name##_ctrl.do_shortcut)) {           \
        HANDLE_SHORTCUT(fn_name, sc_target, ret_t)     \
    } else {                                           \
        ret = MAKE_FN_CALL(real_##fn_name, args);      \
    }
#endif

#define MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, types_and_args, args)     \
    MAKE_SIG(ret_t, fn_name, types_and_args) {                                 \
        struct pt_regs regs;                                                   \
        STORE_REGS();                                                          \
        INITIALIZE_FUNCTION(fn_name, sc_target);                               \
        ingress_work(&fn_name##_ctrl);                                         \
        ret_t ret;                                                             \
        if (do_sc(fn_name##_ctrl.do_shortcut)) {                               \
            HANDLE_SHORTCUT(fn_name, sc_target, ret_t)                         \
        } else {                                                               \
            ret = MAKE_FN_CALL(real_##fn_name, args);                          \
        }                                                                      \
        egress_work(&fn_name##_ctrl);                                          \
        return ret;                                                            \
    }

#if 0
try to substitute
;
            uint64_t user_stack;                                               \

            asm volatile("mov %%rsp, %0" : "=m"(user_stack) : : "memory");     \
            asm volatile("mov  %gs:0x17b90,%rsp");                             \
            asm("movq %%rsp, %0" : "=r"(user_stack) : : "memory");             \


            asm volatile("mov %0, %%rsp" : : "r"(user_stack));                 \


#endif

// This just turns n args into a single arg.
#define COMBINE_ARGS(...) __VA_ARGS__

#define MAKE_STRUCTS_AND_FN_6(fn_name, sc_target, ret_t, t_1, arg_1, t_2,      \
                              arg_2, t_3, arg_3, t_4, arg_4, t_5, arg_5, t_6,  \
                              arg_6)                                           \
    MAKE_STRUCTS(fn_name,                                                      \
                 ret_t (*fn_name##_t)(t_1 arg_1, t_2 arg_2, t_3 arg_3,         \
                                      t_4 arg_4, t_5 arg_5, t_6 arg_6))        \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t,                               \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, \
                                   t_5 arg_5, t_6 arg_6),                      \
                      COMBINE_ARGS(arg_1, arg_2, arg_3, arg_4, arg_5, arg_6))
#define MAKE_STRUCTS_AND_FN_5(fn_name, sc_target, ret_t, t_1, arg_1, t_2,      \
                              arg_2, t_3, arg_3, t_4, arg_4, t_5, arg_5)       \
    MAKE_STRUCTS(fn_name,                                                      \
                 ret_t (*fn_name##_t)(t_1 arg_1, t_2 arg_2, t_3 arg_3,         \
                                      t_4 arg_4, t_5 arg_5))                   \
    MAKE_INTERPOSE_FN(                                                         \
        fn_name, sc_target, ret_t,                                             \
        COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4, t_5 arg_5),   \
        COMBINE_ARGS(arg_1, arg_2, arg_3, arg_4, arg_5))

#define MAKE_STRUCTS_AND_FN_4(fn_name, sc_target, ret_t, t_1, arg_1, t_2,      \
                              arg_2, t_3, arg_3, t_4, arg_4)                   \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t)(t_1 arg_1, t_2 arg_2,           \
                                               t_3 arg_3, t_4 arg_4))          \
    MAKE_INTERPOSE_FN(                                                         \
        fn_name, sc_target, ret_t,                                             \
        COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3, t_4 arg_4),              \
        COMBINE_ARGS(arg_1, arg_2, arg_3, arg_4))

#define MAKE_STRUCTS_AND_FN_3(fn_name, sc_target, ret_t, t_1, arg_1, t_2,      \
                              arg_2, t_3, arg_3)                               \
    MAKE_STRUCTS(fn_name,                                                      \
                 ret_t (*fn_name##_t)(t_1 arg_1, t_2 arg_2, t_3 arg_3))        \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t,                               \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2, t_3 arg_3),           \
                      COMBINE_ARGS(arg_1, arg_2, arg_3))

#define MAKE_STRUCTS_AND_FN_2(fn_name, sc_target, ret_t, t_1, arg_1, t_2,      \
                              arg_2)                                           \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t)(t_1 arg_1, t_2 arg_2))          \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t,                               \
                      COMBINE_ARGS(t_1 arg_1, t_2 arg_2),                      \
                      COMBINE_ARGS(arg_1, arg_2))

#define MAKE_STRUCTS_AND_FN_1(fn_name, sc_target, ret_t, t_1, arg_1)           \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t)(t_1 arg_1))                     \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, COMBINE_ARGS(t_1 arg_1),      \
                      COMBINE_ARGS(arg_1))

#define MAKE_STRUCTS_AND_FN_0(fn_name, sc_target, ret_t)                       \
    MAKE_STRUCTS(fn_name, ret_t (*fn_name##_t)())                              \
    MAKE_INTERPOSE_FN(fn_name, sc_target, ret_t, COMBINE_ARGS(), COMBINE_ARGS())

#endif
