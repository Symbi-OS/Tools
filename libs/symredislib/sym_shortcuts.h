
#ifndef __SYM_SHORTCUTS_H
#define __SYM_SHORTCUTS_H

#include <stdint.h>
#include <sys/uio.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define SYM_CACHE_SZ 1000
#define MSG_STRUCT_SZ 96
#define FIRST_FIELDS 0x28
#define THIRD_FIELDS 0x28

typedef int (*my_ksys_write_t)(unsigned int fd, const char *buf, size_t count);
typedef void (*void_fn_ptr)(unsigned long);
typedef int (*my_ksys_read_t)(unsigned int fd, const char *buf, size_t count);

typedef int (*my_tcp_sendmsg_t)(void *, void*, size_t count);
typedef int (*my_tcp_recvmsg_t)(void *, void*, size_t len,
                                int nonblock, int flags, int *addrlen);
typedef unsigned long (*my___fdget_t)(unsigned int);

void_fn_ptr get_fn_address(char *symbol);
//extern uint64_t int3_rdi;
extern uint64_t addr_msg;
extern my_ksys_read_t my_ksys_read;
extern my_ksys_write_t my_ksys_write;
extern my_tcp_sendmsg_t tcp_sendmsg;
extern my_tcp_recvmsg_t tcp_recvmsg;
extern struct cache_elem* sym_cache;

struct kiocb_struct{
  void * file_p;
  char rest[40];
};

struct msg_iter {
	uint8_t iter_type;
	uint8_t data_source;
	uint64_t iov_offset;
	uint64_t count;
	union {
		void *iov;
		void *kvec;
		void *bvec;
		void *xarray;
		void *pipe;
	};
	union {
		unsigned long nr_segs;
		struct {
			unsigned int head;
			unsigned int start_head;
		};
		uint64_t xarray_start;
	};
};

/* static_assert(sizeof(struct msg_iter) == 40, "Size of msg_iter is not correct"); */

struct msg_struct {
  union {
    char raw[MSG_STRUCT_SZ];
    struct {
      void * msg_name;
      uint32_t msg_name_len;
      struct msg_iter mi;
      /* char first_part[FIRST_FIELDS]; */
      char third_part[32];
      struct kiocb_struct *kiocb_p;
    };
  };
};

/* static_assert(sizeof(struct msg_struct) == 96, "Size of msg_struct is not correct"); */

struct sym_net_state{
  void               *sk;
  uint64_t            ctr;

  struct msg_struct   msg;
  struct iovec        iov;
  struct kiocb_struct ks;
};

struct cache_elem{
  struct sym_net_state send;
  struct sym_net_state recv;
  bool valid;
};

void init_tcp_sc();
void init_ksys_sc();
void invalidate_cache_elem(int fd);
void print_msg_iter(struct msg_iter *mi);
void print_kiocb_struct(struct kiocb_struct *ki);
void print_msg_struct(struct msg_struct *msg);
void clear_sym_net_state(struct sym_net_state *net_state);
void clear_cache_elem(struct cache_elem *cache_elem);
void init_sym_net_state(struct sym_net_state *net_state);
void init_cache_elem(struct cache_elem *cache_elem);
void update_sym_net_state(struct sym_net_state *net_state, void *sk, void* file_p);
void update_cache_elem(struct cache_elem *cache_elem, void *sk, void* file_p);
void update_net_state_hot(struct sym_net_state *net_state, const void *buf, int buf_len);
void * fd_to_filep(int fd);
void check_on_probe(uint64_t addr);
int write_path(int fd, const void *data, size_t data_len);
int write_populate_cache(int fd, const void *data, size_t data_len);
int cached_tcp_sendmsg_path(int fd, const void *data, size_t data_len);
int cached_tcp_recvmsg_path(int fd, const void *buf, size_t buf_len);
#endif //__SYM_SHORTCUTS_H
