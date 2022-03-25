#ifndef __VM_TOOL__
#define __VM_TOOL__

struct params{
  void * input_addr;
  bool print_flag;
};

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);
typedef void (*void_fn_ptr)(unsigned long);

#endif
