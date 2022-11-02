.data

.text

.globl _start
_start:

/* syscall sym_mode_switch(unsigned int direction) */
    mov    x8, #448
    mov    x0, #1
    svc    #0x0	

    bl    lower

/* syscall exit(int status) */
    mov     x0, #0      /* status := 0 */
    mov     w8, #93     /* exit is syscall #93 */
    svc     #0          /* invoke syscall */
