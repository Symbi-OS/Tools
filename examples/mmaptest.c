/* mmaptest.c : to build gcc -g -O1 -o mmaptest mmaptest.c */

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <err.h>
#include <sys/mman.h>
#include <stdio.h>

#define PRIVATE_MAPPING

size_t
statFile(char *path)
{
  struct stat sb;

  if (stat(path, &sb) < 0) {
    return -1;
  }
  return  sb.st_size;
}

int
createFile(const char *path, size_t size)
{
  int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

  if (fd < 0) err(1, "open: %s", path);
  if (lseek(fd, size-1, SEEK_SET) < 0) err(1, "lseek: %s", path);
  if (write(fd, "", 1) != 1) err(1, "write: %s", path);

  return fd;
}

int
openFile(const char *file_path)
{
  struct stat sb;
  int fd;

  fd  = open(file_path, O_RDWR | O_CLOEXEC);
  if (fd < 0) err(1, "open: %s", file_path);
  return fd;
}

int
mapFile(int fd, size_t len, int mmapFlags, void **addr)
{

  *addr = mmap(NULL, // addr
	       len, // length,
	       PROT_READ | PROT_WRITE, // prot
	       mmapFlags,
	       fd, // fd,
	       0);  // offset

  if (*addr == MAP_FAILED) err(1, "mmap: of guest memory failed");

  return 1;
}

int
unmapAddr(void *addr, size_t len)
{
  if (munmap(addr, len) < 0) err(2, "ERROR: SVunmap: munmap");
  return 1;
}

int main(int argc, char **argv)
{
  char *file = NULL;
  size_t len;
  void *addr = NULL;
  int  mmapFlags = 0;
  int fd;


  if (argc < 2) {
    fprintf(stderr, "USAGE: <len> [file]\n"
	    " Simple utility to create a mmapped region and touch write each byte\n"
	    " If no file is specified then an anonymous region is created (no named backing file)\n"
	    " If a non-existent file is specified it is created (with specified len) and used for the mapping\n"
	    " If a existing file is specified it used for the mapping and len is overriden with file length\n");
    return -1;
  }


  len = atol(argv[1]);


  if (argc>2) {
    file = argv[2];
  }

  if (file == NULL) {
    // no file specified create an anonymous mapping (not backed by a named file)
    mmapFlags |= MAP_ANONYMOUS;
    fd = -1;
  } else {
    // check if file exists
    size_t flen = statFile(file);
    if (flen == -1) {
      // file does not exit -- create it
      fd = createFile(file, len);
    } else {
      // file exists let is size override what was specified
      len = flen;
      fprintf(stderr, "%s: exits with length %ld overrides specified len\n", file, len);
      fd = openFile(file);
    }
  }

  // FIXME: Make some of these commandline switches to control
#ifdef PRIVATE_MAPPING
  mmapFlags |=  MAP_PRIVATE;
#else
  mmapFlags |=   MAP_SHARED;
#endif

  mapFile(fd, len, mmapFlags, &addr);

  fprintf(stderr, "Settings: file=%p len=%ld fd=%d mmapFlags=%x addr=%p\n", file, len, fd, mmapFlags, addr);

  unsigned long long num = len/sizeof(unsigned long long);

  for (unsigned long long i=0; i<num; i++) {
    ((unsigned long long *)addr)[i] += i;
    if (((i*sizeof(unsigned long long)) % 4096)==0) fprintf(stderr, ".");
  }
  fprintf(stderr, "\n");


  unmapAddr(addr, len);
  close(fd);


  return 0;
}
