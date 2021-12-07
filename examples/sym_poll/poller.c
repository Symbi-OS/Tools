#include<stdio.h>
/* #include<string.h>	//strlen */
/* #include<sys/socket.h> */
/* #include<arpa/inet.h>	//inet_addr */
/* #include<unistd.h>	//write */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdbool.h>
#include "../../include/headers/sym_all.h"
typedef void (*virtqueue_disable_cb_t)(uint64_t vq);
typedef bool (*virtqueue_enable_cb_t)(uint64_t vq);

typedef bool (*napi_complete_done_t)(uint64_t napi, int work_done);

/* rq* - vq  */
/* napi */


typedef int (*virtnet_poll_t)(uint64_t napi, int budget);

int virtnet_poll(uint64_t napi, int budget){
  // virtqueue disable cb
  virtnet_poll_t virtnet_poll = (virtnet_poll_t) 0xffffffff817656a0;
  return virtnet_poll(napi, budget);
}

bool virtqueue_enable_cb(uint64_t vq){
  // virtqueue disable cb
  virtqueue_enable_cb_t vq_enable_cb = (virtqueue_enable_cb_t) 0xffffffff814f7090;
  return vq_enable_cb(vq);
}

void virtqueue_disable_cb(uint64_t vq){
  // virtqueue disable cb
  virtqueue_disable_cb_t vq_dis_cb = (virtqueue_disable_cb_t) 0xffffffff814f6cc0;
  vq_dis_cb(vq);
}

bool disable_napi(uint64_t napi, int work_done){
  napi_complete_done_t napi_comp_done = (napi_complete_done_t) 0xffffffff819380f0;
  return napi_comp_done(napi, work_done);
}

int main(int argc , char *argv[])
{
  if(argc != 5){
    fprintf(stderr, "expected ./poller <napi> <vq> <processed> <cb_enable> \n");
  }
  /* unsigned long long addr = strtoull(argv[1], NULL, 16); */
  uint64_t napi = strtoull(argv[1], NULL, 16) ;
  uint64_t vq   = strtoull(argv[2], NULL, 16) ;
  int processed = atoi(argv[3]); // strtoull(argv[3], NULL, 16);

  int cb_enable = atoi(argv[4]);

  napi = 0xffff88800352c008;
  vq = 0xffff88800443c240;
  processed = 0;

  printf("got %#llx \n", napi);
  printf("got %#llx \n", vq);
  printf("got %d \n", processed);

  sym_touch_every_page_text();
  sym_elevate();

  if (cb_enable == 0){
    virtqueue_disable_cb(vq);
    bool rc = disable_napi(napi, processed); //  napi, processed);
    printf("disable got %d\n", rc);
  } else if(cb_enable == 1){
    printf("enable\n");
    virtqueue_enable_cb(vq);
  } else if(cb_enable == 2) {
    printf("poll %d\n", virtnet_poll(napi, 100));
  }

  sym_lower();
	return 0;
}
