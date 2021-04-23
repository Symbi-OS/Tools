#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "../lib_constructors/elevate.h"

/* #include "../../linux/include/linux/file.h" */
/* #include "../../linux/include/linux/fs.h" */

#include "kallsymlib.h"
// Really should include this from linux headers.
struct fd {
	struct file *file;
	unsigned int flags;
};

int NUM_REPS = 1<<23;
/* int NUM_REPS = 1<<30; */

/* void handle_sigint(int sig) */
/* { */
/*   fprintf(stderr, "Caught signal %d\n", sig); */
/*   fprintf(stderr, "About to do lower in sig handler\n"); */

/*   sym_lower(); */

/*   fprintf(stderr, "Done with lower\n"); */
/*   exit(0); */
/* } */



typedef int(*ksys_write_type)(unsigned int fd, const char *buf, size_t count);
/* typedef int(*vfs_write_type)(unsigned int fd, const char *buf, size_t count); */

typedef ssize_t (*vfs_write_type)(struct file *file, const char *buf, size_t count, loff_t *pos);

typedef unsigned long (*__fdget_pos_type)(unsigned int fd);

typedef void (*void_fn_ptr)();

void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return (void_fn_ptr) info->addr;
}

static inline struct fd __to_fd(unsigned long v)
{
	return (struct fd){(struct file *)(v & ~3),v & 3};
}

static inline struct fd fdget_pos(int fd)
{
  // Get fn ptr to __fd_get_pos
  __fdget_pos_type my___fdget_pos  = (__fdget_pos_type) get_fn_address("__fdget_pos");
  assert(my___fdget_pos != NULL);
	return __to_fd(my___fdget_pos(fd));
}


typedef ssize_t (*vfs_write_type)(struct file *file, const char *buf, size_t count, loff_t *pos);

static inline loff_t *file_ppos(struct file *file)
{
	return file->f_mode & FMODE_STREAM ? NULL : &file->f_pos;
}

ssize_t local_ksys_write(unsigned int fd, const char *buf, size_t count)
{
  /* file_ppos_type my_file_ppos  = (file_ppos_type) get_fn_address("file_ppos"); */
  /* assert(my_file_ppos != NULL); */

  struct fd f = fdget_pos(fd);
  ssize_t ret = -9;

  if (f.file) {
    loff_t pos, *ppos = file_ppos(f.file);
    if (ppos) {
      pos = *ppos;
      ppos = &pos;
    }
    ret = vfs_write(f.file, buf, count, ppos);
    if (ret >= 0 && ppos)
      f.file->f_pos = pos;
    fdput_pos(f);
  }

  return ret;
}



#define WR_SYSCALL_NUM 1
void syscall_loop(int reps){
  assert(reps > 0);

  int fd = 1;
  const void *buf = "Tommy!\n";
  size_t size = strlen(buf);

  while(reps--){
      register int64_t rax __asm__ ("rax") = WR_SYSCALL_NUM;
      register int rdi __asm__ ("rdi") = fd;
      register const void *rsi __asm__ ("rsi") = buf;
      register size_t rdx __asm__ ("rdx") = size;
      __asm__ __volatile__ (
                            "syscall"
                            : "+r" (rax)
                            : "r" (rdi), "r" (rsi), "r" (rdx)
                            : "rcx", "r11", "memory"
                            );
  }

}

void skipping_syscall_instruction(int reps){
  assert(reps > 0);

  while(reps--){
    register int    syscall_no  asm("rax") = 1; // write
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;

    // Get r11 setup
    asm("pushfq");
    asm("popq %r11"); // let compiler know we're clobbering r11

    // Get rflags setup
    asm("cli");

    // Get RCX setup this is the retrun instruction.
    asm("mov $0x401de7, %rcx"); // specify clobber here

    asm("jmp 0xffffffff81c00010"); // lstar pointer to system call handler
  }
}

static ksys_write_type my_ksys_write = NULL;
/* my_ksys_write = (ksys_write_type) 0xffffffff8133e990; */

void ksys_write_shortcut(int reps, ksys_write_type my_ksys_write){
  assert(reps > 0);
  assert(my_ksys_write != NULL);

	while(reps--){
    //		if( (count % (1<<10)) == 0) {
    /* write(1, "Opportunity to catch a signal\n", 30); */
    //		} else {
    my_ksys_write(1, "Tommy\n", 6);
    /* fprintf("Tommy\n"); */
    //		}
	}
  /* kallsymlib_cleanup(); */
}

void vfs_write_shortcut(int reps, ksys_write_type my_vfs_write){
  assert(reps > 0);
  assert(my_ksys_write != NULL);

	while(reps--){
    //		if( (count % (1<<10)) == 0) {
    /* write(1, "Opportunity to catch a signal\n", 30); */
    //		} else {
    my_ksys_write(1, "Tommy\n", 6);
    /* fprintf("Tommy\n"); */
    //		}
	}
  /* kallsymlib_cleanup(); */
}

int main(){
  /* signal(SIGINT, handle_sigint); */


#ifdef STATIC_BUILD
  sym_elevate();
#endif


  clock_t start, end;
  double cpu_time_used;

  ksys_write_type my_ksys_write = get_fn_address("ksys_write");
  vfs_write_type my_vfs_write = get_fn_address("vfs_write");

  start = clock();

  ksys_write_shortcut(NUM_REPS, my_ksys_write);

  vfs_write_shortcut(NUM_REPS, my_ksys_write);

  /* syscall_loop(NUM_REPS); */

  end = clock();

#ifdef STATIC_BUILD
  sym_lower();
#endif
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
  fprintf(stderr, "Time used: %f\n", cpu_time_used);

  return 0;
}
