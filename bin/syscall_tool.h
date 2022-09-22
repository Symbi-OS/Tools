#ifndef SYSCALL_TOOL_H_
#define SYSCALL_TOOL_H_

extern void _interposer(void);

struct params{
  // are we printing the current syscall table pointer
  bool get_flag;
  // are we modifying the syscall table pointer
  bool mod_flag;
  // address to write to the msr if we are modifying
  bool insert_flag;
  void * mod_addr;
};

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);
typedef void (*void_fn_ptr)(unsigned long);

#endif // SYSCALL_TOOL_H_
