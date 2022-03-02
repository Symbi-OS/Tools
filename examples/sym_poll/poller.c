#include<stdio.h>
/* #include<string.h>	//strlen */
/* #include<sys/socket.h> */
/* #include<arpa/inet.h>	//inet_addr */
/* #include<unistd.h>	//write */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdbool.h>
#include "LINF/sym_all.h"
typedef void (*virtqueue_disable_cb_t)(uint64_t vq);
typedef bool (*virtqueue_enable_cb_t)(uint64_t vq);

typedef bool (*napi_complete_done_t)(uint64_t napi, int work_done);

/* rq* - vq  */
/* napi */


typedef int (*virtnet_poll_t)(uint64_t napi, int budget);

int virtnet_poll(uint64_t napi, int budget){
  virtnet_poll_t virtnet_poll = (virtnet_poll_t) 0xffffffff818eda30;
  fprintf(stderr, "%s: [%p] napi= %lx budget= %d\n", __func__, virtnet_poll, napi, budget );
  return virtnet_poll(napi, budget);
}

/**
 * virtqueue_enable_cb - restart callbacks after disable_cb.
 * @_vq: the struct virtqueue we're talking about.
 *
 * This re-enables callbacks; it returns "false" if there are pending
 * buffers in the queue, to detect a possible race between the driver
 * checking for more work, and enabling callbacks.
 *
 * Caller must ensure we don't call this with other virtqueue
 * operations at the same time (except where noted).
 */
bool virtqueue_enable_cb(uint64_t vq){
  virtqueue_enable_cb_t vq_enable_cb = (virtqueue_enable_cb_t) 0xffffffff817b7bb0;
  fprintf(stderr, "%s: [%p] vq= %lx\n", __func__, vq_enable_cb, vq );
  return vq_enable_cb(vq);
}

/**
 * virtqueue_disable_cb - disable callbacks
 * @_vq: the struct virtqueue we're talking about.
 *
 * Note that this is not necessarily synchronous, hence unreliable and only
 * useful as an optimization.
 *
 * Unlike other operations, this need not be serialized.
 */
void virtqueue_disable_cb(uint64_t vq){
  // virtqueue disable cb
  virtqueue_disable_cb_t vq_dis_cb = (virtqueue_disable_cb_t) 0xffffffff817b7690;
  fprintf(stderr, "%s: [%p] vq= %lx\n", __func__, vq_dis_cb, vq );
  vq_dis_cb(vq);
}

bool disable_napi(uint64_t napi, int work_done){
  napi_complete_done_t napi_comp_done = (napi_complete_done_t) 0xffffffff81a4c080;
  fprintf(stderr, "%s: [%p] napi= %lx work_done= %d\n", __func__, napi_comp_done, napi, work_done );
  return napi_comp_done(napi, work_done);
}

int main(int argc , char *argv[])
{
  if(argc != 5){
    fprintf(stderr, "expected ./poller <napi> <vq> <processed> <cb_enable> \n");
    exit(-1);
  }
  /* unsigned long long addr = strtoull(argv[1], NULL, 16); */
  uint64_t napi = strtoull(argv[1], NULL, 16) ;
  uint64_t vq   = strtoull(argv[2], NULL, 16) ;
  int processed = atoi(argv[3]); // strtoull(argv[3], NULL, 16);

  int cb_enable = atoi(argv[4]);

  fprintf(stderr, "got %#lx \n", napi);
  printf("got %#lx \n", vq);
  printf("got %d \n", processed);
  printf("got %d \n", cb_enable);

  sym_touch_every_page_text();
  sym_touch_stack();
  sym_elevate();

  if (cb_enable == 0){
    virtqueue_disable_cb(vq);
    bool rc = disable_napi(napi, processed); //  napi, processed);
    printf("disable got %d\n", rc);
  } else if(cb_enable == 1){
    printf("enable\n");
    virtqueue_enable_cb(vq);
  } else if(cb_enable == 2) {
    printf("poll %d\n", virtnet_poll(napi, -1));
  }

  sym_lower();
	return 0;
}
