#ifndef __IDT_TOOL__
#define __IDT_TOOL__

#define NUM_IDT_ENTRIES 256

enum mod_options {
  MOD_IST_ENABLE,
  MOD_IST_DISABLE,
  MOD_ADDR
};

enum handler_options {
  HDL_DF,
  HDL_TF,
  HDL_I3,
  HDL_DB
};

struct params{
  // This should be initialized before use.
  // See init_params

  // The relevant IDT
  struct dtr idt;

  // Just return current idtr
  bool get_idtr_flag;

  // User supplied IDT
  void * input_idt;

  // Descriptor offset in IDT
  int vector;

  // We're modifying a descriptor
  bool mod_flag;
  // Which modification we are doing
  int mod_option;
  // This is the address of the handler we're installing
  void * mod_addr;

  // Are we just printing?
  bool print_flag;

  // Need to copy IDT
  bool cp_flag;

  // Installing another IDT
  bool install_flag;

  // Get a handler page
  bool hdl_flag;

  // What handler to use
  int hdl_option;

};

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);
typedef void (*void_fn_ptr)(unsigned long);

#endif
