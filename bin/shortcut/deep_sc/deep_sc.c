#define _GNU_SOURCE
#ifdef DEEP_SHORTCUT
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include "LINF/sym_all.h"  //fatal error no such LIDK/idk.h
#include "deep_sc.h"

my_ksys_read_t my_ksys_read;
my_ksys_write_t my_ksys_write;
my___fdget_t my___fdget = ((void*)0);

void init_tcp_sc(){
  tcp_sendmsg = NULL;
  tcp_recvmsg = NULL;
  sym_cache = (struct cache_elem*) calloc(SYM_CACHE_SZ, (sizeof(struct cache_elem)));
}

void init_ksys_sc(){
  my_ksys_read = NULL;
  my_ksys_write = NULL;
}

void invalidate_cache_elem(int fd){
  sym_cache[fd].send.valid = false;
  sym_cache[fd].recv.valid = false;
}

void print_msg_iter(struct msg_iter *mi){
  fprintf(stderr, "mi->data_source %u\n", mi->data_source);
  fprintf(stderr, "mi->count %lx\n", mi->count);
  fprintf(stderr, "mi->iov %p\n", mi->iov);
}

void print_kiocb_struct(struct kiocb_struct *ki){
  fprintf(stderr, "ki->file_p %p\n", ki->file_p);
}

void print_msg_struct(struct msg_struct *msg){
  fprintf(stderr, "\n");
  fprintf(stderr, "name %p\n",    msg->msg_name );
  fprintf(stderr, "len %d\n",     msg->msg_name_len);
  fprintf(stderr, "kiocb_p %p\n",(void *) msg->kiocb_p);


  print_msg_iter(&msg->mi);
  print_kiocb_struct(msg->kiocb_p);
}

void clear_sym_net_state(struct sym_net_state *net_state){
  memset(&net_state->msg, 0, sizeof(struct msg_struct));
  memset(&net_state->iov, 0, sizeof(struct iovec));
  memset(&net_state->ks,  0, sizeof(struct kiocb_struct));
}

void clear_cache_elem(struct cache_elem *cache_elem){
  clear_sym_net_state(&cache_elem->send);
  clear_sym_net_state(&cache_elem->recv);
}

void init_sym_net_state(struct sym_net_state *net_state){
  // Hook iov into msg's msg_iter
  net_state->msg.mi.iov = &(net_state->iov);
  // Hook kiocb into msg's kiocb pointer
  net_state->msg.kiocb_p = &(net_state->ks);
  // NOTE sk and ctr are 0 from memset
  net_state->valid = true;
}

void update_sym_net_state(struct sym_net_state *net_state, void *sk, void* file_p){
  net_state->sk = sk;

  // put file pointer into msg's kiocb ptr.
  net_state->ks.file_p = file_p;
  // NOTE sk and ctr are 0 from memset
}

void update_cache_elem(struct cache_elem *cache_elem, void *sk, void* file_p){
  // Got socket from BP exception
  update_sym_net_state(&cache_elem->send, sk, file_p);
  update_sym_net_state(&cache_elem->recv, sk, file_p);
}

void update_net_state_hot(struct sym_net_state *net_state, const void *buf, int buf_len){
  // iov will get loaded on hot path
  // msg_iter.count
  net_state->iov.iov_base = (void *) buf;
  net_state->iov.iov_len = buf_len;
  net_state->msg.mi.count = buf_len;
}

void * fd_to_filep(int fd){
  if (my___fdget == NULL) {
      my___fdget = (my___fdget_t) sym_get_fn_address("__fdget");
      sym_elevate();
  }
  uint64_t tmp = my___fdget(fd);
  tmp >>= 1;
  tmp <<= 1;
  return (void *) tmp;
}

void check_on_probe(uint64_t addr){
  printf("1st byte at probe is %#x \n", *((unsigned char *) addr));
}

int write_path(int fd, const void *data, size_t data_len){
  int ret = write(fd, data, data_len);
  return ret;
}

int write_populate_cache(int fd, const void *data, size_t data_len){
  int ret;
  clear_sym_net_state(&sym_cache[fd].send);
  uint64_t reg = 0;
  uint64_t flag = DB_GLOBAL;
  int core = sched_getcpu();
  struct scratchpad * sp = (struct scratchpad *) get_scratch_pg(core);
  sym_set_db_probe((uint64_t)tcp_sendmsg, reg, flag);
  /* sym_elevate(); */
  sp->read_addr_msg = 0;

  // run syscall ... triggers probe.
  ret = real_write(fd, data, data_len);
  // NOW WE HAVE SK AND MSG COPIED
  memcpy(&sym_cache[fd].send.msg, (void *)sp->addr_msg, 96);
  // This stitches iov and ks into msg.
  // again, sym_cache[conn->fd].msg.mi already populated.
  // has some junk kern ptrs that need to be updated.
  // NOTE: don't move this above write().
  init_sym_net_state(&sym_cache[fd].send);
  update_cache_elem(&sym_cache[fd], (void *)(sp->get.pt_r.rdi), fd_to_filep(fd));
  return ret;
}

int read_populate_cache(int fd, const void *data, size_t data_len){
  int ret;
  clear_sym_net_state(&sym_cache[fd].recv);

  uint64_t reg = 0;
  uint64_t flag = DB_GLOBAL;
  int core = sched_getcpu();
  struct scratchpad * sp = (struct scratchpad *) get_scratch_pg(core);
  sym_set_db_probe((uint64_t)tcp_recvmsg, reg, flag);

  /* sym_elevate(); */
  sp->read_addr_msg = 0;
  ret = real_read(fd, (void *)data, data_len);
  memcpy(&sym_cache[fd].recv.msg, (void *)sp->addr_msg, 96);
  init_sym_net_state(&sym_cache[fd].recv);
  update_cache_elem(&sym_cache[fd], (void *)(sp->get.pt_r.rdi), fd_to_filep(fd));
  return ret;
}

int cached_tcp_sendmsg_path(int fd, const void *data, size_t data_len){
  //             ,
  // the fish. <:===<
  //             '
  int ret;
  update_net_state_hot(&sym_cache[fd].send, data, data_len);
  struct sym_net_state local_sns = sym_cache[fd].send;
  init_sym_net_state(&local_sns);
  uint64_t user_stack;
  asm volatile("mov %%rsp, %0" : "=m"(user_stack) : : "memory");
  asm volatile("mov %gs:0x17b90, %rsp");
  ret = tcp_sendmsg(local_sns.sk, &local_sns.msg, data_len);
  asm volatile("mov %0, %%rsp" : : "r"(user_stack));
  return ret;
}

int cached_tcp_recvmsg_path(int fd, const void *buf, size_t buf_len){
  int addr_len = 0;
  int ret;
  update_net_state_hot(&sym_cache[fd].recv, buf, buf_len);
  struct sym_net_state local_sns = sym_cache[fd].recv;
  init_sym_net_state(&local_sns);
  uint64_t user_stack;
  asm volatile("mov %%rsp, %0" : "=m"(user_stack) : : "memory");
  asm volatile("mov %gs:0x17b90, %rsp");
  ret = tcp_recvmsg(local_sns.sk, &local_sns.msg, buf_len, 64, 0, &addr_len);
  asm volatile("mov %0, %%rsp" : : "r"(user_stack));
  return ret;
}
#endif
