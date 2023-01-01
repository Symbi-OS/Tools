#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#ifdef ELEVATED_MODE
    #include "LINF/sym_all.h"
    typedef int(*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
#endif

