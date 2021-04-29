timeout 2 argdist -p $(./get_redis_pid.sh) -C 'p::tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size):void *:sk'
