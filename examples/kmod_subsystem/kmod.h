#ifndef KMOD_H
#define KMOD_H
#include <stdint.h>

extern void __kmod_kprint(const char* msg);

extern int shortcut_tcp_sendmsg(int fd, void *data, size_t len);

#endif
