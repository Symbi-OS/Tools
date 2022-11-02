.text

    .globl lower

lower:
    stp x0, x1, [sp, #-16]
    msr elr_el1, x30
    mov x0, #0x0
    msr spsr_el1, x0
    ldp x0, x1, [sp, #-16]
    eret
