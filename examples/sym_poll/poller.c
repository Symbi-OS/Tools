#include<stdio.h>
/* #include<string.h>	//strlen */
/* #include<sys/socket.h> */
/* #include<arpa/inet.h>	//inet_addr */
/* #include<unistd.h>	//write */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <assert.h>

#include <stdbool.h>
#include "LINF/sym_all.h"
typedef void (*virtqueue_disable_cb_t)(uint64_t vq);
typedef bool (*virtqueue_enable_cb_t)(uint64_t vq);

typedef bool (*napi_complete_done_t)(uint64_t napi, int work_done);

/* rq* - vq  */
/* napi */


typedef int (*virtnet_poll_t)(uint64_t napi, int budget);
int count = 0;

int virtnet_poll(uint64_t napi, int budget){
  // TODO: change all these to use kall sym lib.
  virtnet_poll_t virtnet_poll = (virtnet_poll_t) 0xffffffff818eda30;
  /* int freq = 100; */
  /* if(count % freq == 0){ */
  /*   fprintf(stderr, "%d polls of \n", freq); */
  /*   fprintf(stderr, "%s: [%p] napi= %lx budget= %d\n", __func__, virtnet_poll, napi, budget ); */
  /* } */
  /* count++; */
  sym_elevate();
  int ret = virtnet_poll(napi, budget);
  sym_lower();
  /* fprintf(stderr, "leaving %s, ret is %d\n", __func__, ret); */
  return ret;
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

  sym_elevate();
  bool ret = vq_enable_cb(vq);
  sym_lower();
  return ret;
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
  sym_elevate();
  vq_dis_cb(vq);
  sym_lower();
}

bool disable_napi(uint64_t napi, int work_done){
  napi_complete_done_t napi_comp_done = (napi_complete_done_t) 0xffffffff81a4c080;
  fprintf(stderr, "%s: [%p] napi= %lx work_done= %d\n", __func__, napi_comp_done, napi, work_done );

  sym_elevate();
  bool ret = napi_comp_done(napi, work_done);
  sym_lower();
  return ret;
}
void help(){
  printf("./poller \n");
  printf("options: pick one of\n");
  printf("\t-d disable: supply -n -v\n");
  printf("\t-e enable:  supply -v\n");
  printf("\t-p poll:    one shot poll: supply -n\n");
  printf("\t-c continuous poll: supply -n -s -i\n");

  printf("\nparameters\n");
  printf("\t-n napi struct pointer\n");
  printf("\t-v vq struct pointer\n");
  printf("\t-s sleep time (ms)\n");
  printf("\t-i iterations\n");
}
int main(int argc , char *argv[])
{
  help();

  uint64_t napi = 0; // = strtoull(argv[1], NULL, 16) ;
  uint64_t vq = 0; //   = strtoull(argv[2], NULL, 16) ;
  int iterations = 0; // = atoi(argv[5]);
  int sleep_time = 0; // = atoi(argv[6]);

  // Only one should be selected
  int disable, enable, poll, cont_poll;
  disable = enable = poll = cont_poll = 0;

  opterr = 0;

  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "depcn:v:q:s:i:")) != -1)
    switch (c)
      {
      case 'd':
        disable = 1;
        break;
      case 'e':
        enable = 1;
        break;
      case 'p':
        poll = 1;
        break;
      case 'c':
        cont_poll = 1;
        break;
      case 'n':
        napi = strtoull(optarg, NULL, 16) ;
        break;
      case 'v':
        vq = strtoull(optarg, NULL, 16) ;
        if(disable && napi){
          virtqueue_disable_cb(vq);
          (void) disable_napi(napi, 0); //  napi, processed);
        }
        // XXX
        if(enable){
          // Think enable is currently broken
          printf("Enable vq %lx\n", vq);
          virtqueue_enable_cb(vq);
        }
        break;
      case 'q':
        break;
      case 's':
        sleep_time = atoi(optarg);
        break;
      case 'i':
        iterations = atoi(optarg);
        break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        /* else if (isprint (optopt)) */
        /*   fprintf (stderr, "Unknown option `-%c'.\n", optopt); */
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

  int index;
  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);

  // allow for example disabling napi and vqs and immediately starting polling
  assert( (disable + enable + poll + cont_poll) >= 1 );

  printf("disable %d, enable %d, poll %d, cont_poll %d\n", disable, enable, poll, cont_poll);
  printf("napi %lx, vq %lx, sleep_time %d, iterations %d\n", napi, vq, sleep_time, iterations );

  // Error checking
  if(disable){
    assert((napi != 0) && (vq != 0));
    /* assert(sleep_time + iterations == 0); */
    assert(enable != 1);

  } else if(enable){
    assert(vq != 0);

    // Don't do anything else
    assert(sleep_time == 0);
    assert(iterations == 0);
    assert(disable == 0);
    assert(poll == 0);
    assert(cont_poll == 0);

  } else if(poll){
    assert(napi != 0);

    assert(sleep_time == 0);
    assert(iterations == 0);
    assert(cont_poll == 0);
  }

  if(cont_poll){
    assert(napi != 0);
    assert( (iterations != 0) && (sleep_time != 0) );
    assert(poll==0);
  }

  // Prepare for symbiote mode
  sym_touch_every_page_text();
  sym_touch_stack();

  if (disable){
    // Already done in loop
    /* virtqueue_disable_cb(vq); */
    /* bool rc = disable_napi(napi, 0); //  napi, processed); */

    /* printf("disable got %d\n", rc); */

  } else if(enable){
    /* printf("enable\n"); */

  } else if(poll) {
    (void) virtnet_poll(napi, -1);
  }

  if(cont_poll) {

    // Special case poll forever...
    if(iterations == -1){
      while(1){
        virtnet_poll(napi, -1);
        sleep(sleep_time);
      }
    } else {
      sym_elevate();
      for(int i=0; i<iterations; i++){
        virtnet_poll(napi, -1);
        usleep(sleep_time);
      }
      sym_lower();
    }

  }

  fprintf(stderr, "leaving %s\n", __func__);
	return 0;
}
