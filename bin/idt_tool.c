#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "../libs/symlib/include/LINF/sym_all.h"

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

void help(){
  eprintf("./idt_tool hi");
  eprintf("options: \n");
  eprintf("\t-h help: this help menu\n");
  eprintf("\t-i idt:  print current idt \n");

  eprintf("\nparameters\n");
}

void get_current_idtr(struct dtr * idt){
  sym_store_idt_desc(idt);
}

void i_print_idtr(){
  struct dtr my_idt;
  get_current_idtr(&my_idt);

  printf("IDT base:  %lx\n", my_idt.base);
  printf("IDT limit: %#x\n", my_idt.limit);
}
void d_print_desc(unsigned int vector){
  struct dtr my_idt;
  get_current_idtr(&my_idt);
  sym_print_idt_desc((unsigned char *) my_idt.base, vector);
}

int main(int argc, char **argv){
  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "d:hi")) != -1)
    switch (c)
      {
      case 'd':
        // Specify offset in dec
        int vector = strtoull(optarg, NULL, 10);
        d_print_desc(vector);
        break;
      case 'h':
        help();
        return 0;
        break;
      case 'i':
        i_print_idtr();
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
        return 1;
      default:
        abort ();
      }



}
