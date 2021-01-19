#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 31
struct sock *nl_sk = NULL;
/*
struct nlmsghdr {
__u32 nlmsg_len; 整个netlink消息的长度，包含消息头。
__u16 nlmsg_type; 消息状态，内核在include / uapi / linux / netlink.h中定义以下4种通用的消息类型。
__u16 nlmsg_flags; 消息标记，它们泳衣表示消息的类型。
__u32 nlmsg_seq;  消息序列号，用以将消息排队有些类似TCP协议中的序号(不完全一样)，但是netlink的这个字段是可选的，不强制使用。
__u32 nlmsg_pid;  发送端口的ID号，对于内核来说该值就是0，对于用户进程来说就是其socket所绑定的ID号。
};
*/
/*
struct sk_buff结构是Linux网络代码中重要的数据结构，它管理和控制接收或发送数据包的信息。
*/

static void hello_nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int pid;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr*)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char*)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0); // 分配一个新的netlink消息 (payload大小,flags内存类型)
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); // 填写头部信息
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid); // 向应用层发送单播消息
    /* nlmsg_unicast为netlink_unicast的封装
    static inline int nlmsg_unicast(struct sock *sk, struct sk_buff *skb, u32 portid) {
        int err = netlink_unicast(sk, skb, portid, MSG_DONTWAIT);
        if (err > 0)
        err = 0;
        return err;
        }
        */
    if (res < 0) {
        printk(KERN_INFO "Error while sending bak to user\n");
    }
}

static void hello_nl_recv_msg1(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int pid;
    int msg_size;
    char *msg = "Hello, this is from kernel modo info.";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    nlh = (struct nlmsghdr*)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char*)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */
    printk(KERN_INFO "Kenerl get pid value is: %d.\n", pid);

    // 创建sk_buff空间
    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0); // 分配一个新的netlink消息 (payload大小,flags内存类型)
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    // 设置消息头部
    nlh = nlmsg_put(skb_out, 0, 0, NETLINK_USER, msg_size, 0); // 填写头部信息
    if (nlh == NULL) {
        printk(KERN_ERR "Error to generate netlink head info.\n");
    }
    //NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);
    res = netlink_unicast(nl_sk, skb_out, pid, MSG_DONTWAIT); // 向应用层发送单播消息
    //res = nlmsg_unicast(nl_sk, skb_out, pid); // 向应用层发送单播消息
    /* nlmsg_unicast为netlink_unicast的封装
    static inline int nlmsg_unicast(struct sock *sk, struct sk_buff *skb, u32 portid) {
        int err = netlink_unicast(sk, skb, portid, MSG_DONTWAIT);
        if (err > 0)
        err = 0;
        return err;
        }
        */
    if (res < 0) {
        printk(KERN_INFO "Error while sending bak to user\n");
    }
}

static int __init hello_init(void) {
    printk("Entering: %s\n", __FUNCTION__); //This is for 3.6 kernels and above.
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg1,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    //nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg,NULL,THIS_MODULE);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void) {

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init); 
module_exit(hello_exit);
MODULE_LICENSE("GPL");
