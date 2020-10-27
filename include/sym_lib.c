void sym_elevate();
void sym_lower();

void sym_elevate(){
  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "elevate";
  register int    arg3        asm("rdx") = 7;
  asm("syscall");
}
void sym_lower(){
  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "lower";
  register int    arg3        asm("rdx") = 5;
  asm("syscall");
}
