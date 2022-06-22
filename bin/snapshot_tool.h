#ifndef SNAPSHOT_TOOL_H
#define SNAPSHOT_TOOL_H

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

uint64_t phys_to_virt(void * phys_addr){
  off_t offset = (off_t)phys_addr;

  // Truncate offset to a multiple of the page size, or mmap will fail.
  size_t pagesize = sysconf(_SC_PAGE_SIZE);
  off_t page_base = (offset / pagesize) * pagesize;
  off_t page_offset = offset - page_base;

  int fd = open("/dev/mem", O_SYNC);
  unsigned char *mem = mmap(NULL, page_offset, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, page_base);
  if (mem == MAP_FAILED) {
    perror("CAN'T MAP MEMORY");
    return (uint64_t) NULL;
  }
  return (uint64_t) mem;
}

#endif
