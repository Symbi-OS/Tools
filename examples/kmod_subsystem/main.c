#include <LINF/sym_all.h>
#include <stdio.h>
#include "kmod.h"

int main() {
    sym_elevate();
    __kmod_kprint("Hello from kernel world!\n");
    sym_lower();
    return 0;
}
