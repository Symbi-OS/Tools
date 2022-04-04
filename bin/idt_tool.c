#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "../libs/symlib/include/LINF/sym_all.h"

#include "idt_tool.h"

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

// XXX all this needs to get pulled into symbiote lib

// TODO: data structure jmp table fixed pos for each symbol

// TODO: GDB for kern code with 0xcc
// TODO: TLB SHOOTDOWN tool
// TODO: cacheline flush

struct ef{
  uint64_t ec;
  uint64_t ip;
  uint64_t cs;
  uint64_t rf;
  uint64_t sp;
  uint64_t ss;
};
#define MY_PUSH_REGS                            \
  __asm__("\
  pushq   %rdi		/* pt_regs->di */ \n\t\
  pushq   %rsi		/* pt_regs->si */ \n\t\
  pushq	  %rdx		/* pt_regs->dx */ \n\t\
  pushq   %rcx		/* pt_regs->cx */ \n\t\
  pushq   %rax		/* pt_regs->ax */ \n\t\
  pushq   %r8		/* pt_regs->r8 */ \n\t\
  pushq   %r9		/* pt_regs->r9 */ \n\t\
  pushq   %r10		/* pt_regs->r10 */ \n\t\
  pushq   %r11		/* pt_regs->r11 */ \n\t\
  pushq	  %rbx		/* pt_regs->rbx */ \n\t\
  pushq	  %rbp		/* pt_regs->rbp */ \n\t\
  pushq	  %r12		/* pt_regs->r12 */ \n\t\
  pushq	  %r13		/* pt_regs->r13 */ \n\t\
  pushq	  %r14		/* pt_regs->r14 */ \n\t\
  pushq	  %r15		/* pt_regs->r15 */ \
");
#define MY_POP_REGS                             \
  __asm__("\
	popq %r15 \n\t\
	popq %r14 \n\t\
	popq %r13 \n\t\
	popq %r12 \n\t\
	popq %rbp \n\t\
	popq %rbx \n\t\
	popq %r11 \n\t\
	popq %r10 \n\t\
	popq %r9  \n\t\
	popq %r8  \n\t\
	popq %rax \n\t\
	popq %rcx \n\t\
	popq %rdx \n\t\
	popq %rsi \n\t\
	popq %rdi \
");

#define MAKE_FAKE_EF                             \
  __asm__("\
                   /*Pretend excep frame*/                              \
  pushq   $0xbad5                                                \n\t   \
  pushq   $0xbad4                                                \n\t   \
  pushq   $0xbad3                                                \n\t   \
  pushq   $0xbad1                                                \n\t   \
  pushq   $0xbad1                                                \n\t   \
  pushq   $0xbad0                                                 \n\t  \
");

#define DEFINE_TF_INTERPOSER \
__asm__("\
                  /*prologue*/                                       \
  .text                                                          \n\t\
  .align 16                                                      \n\t\
  .globl \t tf_interposer_asm                                    \n\t\
  tf_interposer_asm:                                             \n\t\
");

extern void tf_interposer_asm();
// NOTE Define fn in assembly
DEFINE_TF_INTERPOSER

// NOTE Produce fake exception frame
/* MAKE_FAKE_EF */

// NOTE Want to pass ef pointer to interposer C code
__asm__(" \
  pushq %rdi                 /*Preserve rdi */ \n\t\
  movq %rsp, %rdi            /*Get rsp for 1st arg to c fn */ \n\t\
  add $8, %rdi               /* Push set us back 8 */ \n\t");

// NOTE Save all regs.
MY_PUSH_REGS

// NOTE Call into C code
__asm__("  call sym_tf_set_user_bit");

// NOTE Restore regs
MY_POP_REGS

__asm__("                     \
  popq %rdi                   /*Done with 1st arg, restore user rdi  */ \n\t\
\
  push   %rax                      \n\t\
  mov    $0xffffffff81e00ac0,%rax  /* asm_exc_page_fault */ \n\t   \
  xor    (%rsp),%rax               /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp)               \n\t\
  xor    (%rsp),%rax               \n\t\
  ret \
");


// XXX this fn must be on the same page as tf_interposer_asm
void sym_tf_set_user_bit(struct ef * s){
  /* Are we an instruction fetch? */
  if( s->ec & INS_FETCH){
    // We don't need to special case when in ring 3.
    if(! (s->ec & USER_FT) )  {
        // Are we user code?
        // Could look in cr2, but by def rip caused the fault here.
        // This is modeled after kern:fault_in_kernel_space
        if( s->ip < ( (1UL << 47) - PG_SZ) ){
          /// Lie that code was running in user mode.
          s->ec |= USER_FT;
          /* myprintk("swinging err code for\n"); */
          /* print_ef(); */
          /* myprintki("my_ctr %d\n", my_ctr++); */
        }
    }
  }

  /* printf("ss %lx\n", ((struct ef *)s)->ss); */
  /* printf("sp %lx\n", ((struct ef *)s)->sp); */
  /* printf("rf %lx\n", ((struct ef *)s)->rf); */
  /* printf("cs %lx\n", ((struct ef *)s)->cs); */
  /* printf("ip %lx\n", ((struct ef *)s)->ip); */
  /* printf("ec %lx\n", ((struct ef *)s)->ec); */
}

void help(){
  eprintf("./idt_tool:\n");
  eprintf("options:\n");
  eprintf("\t-a <addr>: address of idt, current loaded assumed if not provided\n");
  eprintf("\tc:         copy idt return ptr to copy on kern pg\n");
  eprintf("\tg:         get current idtr\n");
  eprintf("\th:         print this help msg\n");
  eprintf("\ti:         install idt (swing idtr)\n");
  eprintf("\tm <ist_enable|ist_disable|addr:0xaddr>: modify idt entry\n");
  eprintf("\tp:         print\n");
  eprintf("\tv <dec#>:  vector number for print / modify\n");
  eprintf("\tz <df|tf>: which mitigation to copy to kern page\n");

  eprintf("\nexamples:\n");

  eprintf("\ttaskset -c 1 ./idt_tool -g\n");
  eprintf("\ttaskset -c 1 ./idt_tool -c\n");
  eprintf("\t./idt_tool -z tf\n");
  eprintf("\t./idt_tool -a ffffc90000986000 -m addr:0xffffc9000098d000 -v 14\n");
  eprintf("\ttaskset -c 1 ./idt_tool -a ffffc90000986000 -i\n");
}

void get_current_idtr(struct dtr * idt){
  sym_store_idt_desc(idt);
}

void print_idtr(){
  struct dtr my_idt;
  get_current_idtr(&my_idt);

  printf("IDT base:  %lx\n", my_idt.base);
  printf("IDT limit: %#x\n", my_idt.limit);
}

void print_desc(struct dtr *idt, int vector){
  assert(idt->base != 0);
  assert(vector >= 0);

  sym_print_idt_desc((unsigned char *) idt->base, (unsigned int)vector);
}

void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}


void * get_aligned_kern_pg(){

  // TODO: is there a right way to do this?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
  vzalloc_t vzalloc = (vzalloc_t) get_fn_address("vzalloc");
#pragma GCC diagnostic pop 

  sym_elevate();
  void * p = vzalloc(IDT_SZ_BYTES);
  // Mem leak need this to be page aligned 
  sym_lower();

  /* printf("got kern page ptr %p\n", p); */
  assert( ((long unsigned )p % IDT_SZ_BYTES ) == 0);
  return p;
}

void * copy_idt(struct dtr* src_idt){
  // Allocate kern page
  void *idt_cp = get_aligned_kern_pg();

  // Copy idt
  sym_memcpy(idt_cp, (void *) src_idt->base, IDT_SZ_BYTES );

  // Return pointer to page
  return idt_cp;
}

void install_idt(struct dtr *idt){
  sym_load_idtr(idt);
}

void current_or_supplied_idt(struct params *p){
  if(p->input_idt){
    p->idt.base= (uint64_t) p->input_idt;
    p->idt.limit = IDT_SZ_BYTES - 1;
  }else{
    // No user supplied idt
    get_current_idtr(&(p->idt));
  }
}

void idtr_getter(struct params *p){
  get_current_idtr(&(p->idt));
  printf("%lx\n", (p->idt).base);
}

void printer(struct params * p){
  // Printer.
    current_or_supplied_idt(p);

    // If no a flag, use current, otherwise use
    print_desc(&(p->idt), p->vector);
}

void copier(struct params *p){
  // If not a flag, use current, otherwise use
  current_or_supplied_idt(p);

  void * copied_idt = NULL;
  copied_idt = copy_idt(&(p->idt));
  printf("%lx\n", (uint64_t) copied_idt);
}

void installer(struct params *p){
  struct dtr old_idt;
  get_current_idtr(&old_idt);

  // If no a flag, use current, otherwise use
  if(p->input_idt){
    p->idt.base= (uint64_t) p->input_idt;
    p->idt.limit = IDT_SZ_BYTES - 1;
  }else{
    // No user supplied idt
    p->idt = old_idt;
    /* get_current_idtr(&idt); */
  }
  install_idt(&(p->idt));
  printf("%lx\n", old_idt.base);
}

void * modifier(struct params *p){
  // vector not less than 0
  assert(p->mod_option != -1);

  assert( (p->vector >= 0) && (p->vector < NUM_IDT_ENTRIES) );

  current_or_supplied_idt(p);
  // non null idt to modify
  assert((p->idt).base != 0);

  if(p->mod_option == MOD_IST_ENABLE){
    unsigned int enable = 1;
    sym_toggle_pg_ft_ist((unsigned char *) ( (p->idt).base ), enable);

  }else if(p->mod_option == MOD_IST_DISABLE){
    unsigned int disable = 0;
    sym_toggle_pg_ft_ist((unsigned char *) ( (p->idt).base ), disable);

  }else if(p->mod_option == MOD_ADDR){
    assert(p->mod_addr != NULL);

    // Get descriptor ptr
    union idt_desc * desc = sym_get_idt_desc((unsigned char *) (p->idt).base, p->vector);

    sym_update_desc_handler(desc, p->mod_addr);
  }

  printf("%lx\n", (p->idt).base );
  // make sure handler addr isn't 0
  return (void *)(p->idt).base;
}

void sym_flush_tlb(){
  // XXX TEST ME
  sym_elevate();
  __asm__("movq %%cr3, %%rax" :: : "%rax");
  __asm__("movq %rax, %cr3"); // does the flush
  sym_lower();
}

void sym_toggle_page_exe_disable(void * addr, bool disable){
  // When set, XD bit in PTE makes referenced page
  // execute disabled.

  // TODO: We think this only works for kern addrs.
  {
    uint64_t tmp = (uint64_t) addr;
    assert((tmp >> 63) == 1);
  }

  // make it executable
  unsigned int level;
  struct pte *pte = sym_get_pte((uint64_t) addr, &level);

  /* sym_print_pte(pte); */

  sym_elevate(); pte->XD = disable; sym_lower();
  /* sym_print_pte(pte); */

  // TODO Still need to invalidate
  sym_flush_tlb();
}

void handler_pager(struct params *p){
  assert(p->hdl_option != -1);

  // allocate a page for handler
  void * hdl_pg = get_aligned_kern_pg();

  // copy appropriate handler onto it
  void * src = NULL;
  int sz = 0;
  if(p->hdl_option == HDL_DF){
    src = &df_asm_handler;
    sz = 0x1d;
  }
  if(p->hdl_option == HDL_TF){
    /* src = &tf_asm_handler; */
    src = &tf_interposer_asm;
    sz = PG_SZ;
  }

  assert(src != NULL);
  assert(sz != 0);
  assert(sz <= (int) PG_SZ);

  // Do this with a non-temporal store?
  sym_memcpy(hdl_pg, src, sz);

  int disable = 0;
  sym_toggle_page_exe_disable(hdl_pg, disable);

  // return address of page.
  printf("%p\n", hdl_pg);
}


void init_params(struct params * p){
  // Zero struct
  // Apparently this isn't safe on machines that don't use
  // 0 as the NULL vector ... but do you really want to be
  // working on such a machine?
  memset(p, 0, sizeof(struct params));

  // 0 is valid for vector offset, -1 is not
  p->vector = -1;

  p->mod_option = -1;
  p->hdl_option = -1;
}

void parse_args(int argc, char *argv[], struct params *p){

  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "a:cdghilm:pv:z:")) != -1)
    switch (c)
      {
      case 'a':
        // Address
        p->input_idt = (void *) strtoull(optarg, NULL, 16) ;
        break;
      case 'c':
        // Copy
        p->cp_flag = 1;
        break;
      case 'd':
        // Delete
        break;
      case 'g':
        // get idtr
        p->get_idtr_flag = 1;
        break;
      case 'h':
        // help
        help();
        exit(0);
        break;
      case 'i':
        // Install
        p->install_flag = 1;
        break;
      case 'l':
        // List maybe only for shell script
        break;
      case 'm':
        p->mod_flag = 1;

        // Modify
        if(!strcmp(optarg, "ist_enable")){
          p->mod_option = MOD_IST_ENABLE;

        }else if (!strcmp(optarg, "ist_disable")){
          p->mod_option = MOD_IST_DISABLE;

        } else if(! strncmp(optarg, "addr:", 5)){
          // If first 5 chars are "addr:"
          char *c = optarg;
          c += 5;
          p->mod_option = MOD_ADDR;
          p->mod_addr = (void *) strtoull(c, NULL, 16);
        }else {
          eprintf("Don't know that mod option\n");
          exit(-1);
        }
        break;
      case 'p':
        // Print
        p->print_flag = 1;
        break;
      case 'v':
        // Entry

        p->vector = strtoull(optarg, NULL, 10) ;
        // Specify offset in dec
        /* int vector = strtoull(optarg, NULL, 10); */
        /* d_print_desc(vector); */
        break;
      case 'z':
        p->hdl_flag = 1;
        if(!strcmp(optarg, "df")){
          p->hdl_option = HDL_DF;
        }
        if(!strcmp(optarg, "tf")){
          p->hdl_option = HDL_TF;
        }
        break;

      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        /* else if (isprint (optopt)) */
        /*   fprintf (stderr, "Unknown option `-%c'.\n", optopt); */
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        exit(1);
      default:
        abort ();
      }
}

int main(int argc, char *argv[]){
  sym_lib_init();
  /* sym_touch_every_page_text(); */

  /* copy_and_install_idk_kern_page(); */
  // NOTE assumes the existance of at least idt_maps/sys_default.idt

  // Don't operate on this directly, use pointer
  struct params params;

  // For consistency, always operate on pointer
  struct params *p = &params;

  init_params(p);

  parse_args(argc, argv, p);

  // IDTR Getter
  if(p->get_idtr_flag){
    idtr_getter(p);
    return 0;
  }

  // Printer
  if(p->print_flag && (p->vector != -1)){
    printer(p);
  }

  // Copier
  if(p->cp_flag){
    copier(p);
  }

  // Modifier
  if(p->mod_flag && (p->vector != -1)){
    modifier(p);
  }

  // Installer
  if(p->install_flag){
    installer(p);
  }

  // Handler pager
  if(p->hdl_flag){
    handler_pager(p);
  }

  /* printf("done main\n"); */
}
