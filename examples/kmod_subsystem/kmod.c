#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/printk.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <net/sock.h>
#include <net/tcp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anonymous Template");
MODULE_DESCRIPTION("kmod");
MODULE_VERSION("1.0");

void __kmod_kprint(const char* msg) {
    printk(msg);
}

int shortcut_tcp_sendmsg(int fd, void *data, size_t len)
{
	struct iovec iov;
    struct msghdr msg = {0};  // Zeroing all members
    struct fd f;
    struct socket *socket;
    struct sock *sk;
    struct kiocb kiocb;
    loff_t pos, *ppos;
    int ret = 0;

    iov.iov_base = data;
    iov.iov_len = len;

    // Initialize the iterator
    iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, len);

    f = fdget(fd); // Replacing fdget_pos
    ppos = &f.file->f_pos;
    if (ppos) {
        pos = *ppos;
        ppos = &pos;
    }

    socket = (struct socket *)f.file->private_data;
    sk = socket->sk;

    init_sync_kiocb(&kiocb, f.file);
    kiocb.ki_pos = (ppos ? *ppos : 0);
    msg.msg_iocb = &kiocb;

    /* Up until now, this has all been setup, lock the socket and send */
    lock_sock(sk);
    
    ret = tcp_sendmsg_locked(sk, &msg, len);

    release_sock(sk);

    fdput(f); // Replacing fdput_pos

    return ret;
}

int init_module(void) {
    return 0;
}

void cleanup_module(void) {
}
