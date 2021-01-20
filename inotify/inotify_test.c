#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
 
/*    
 * 指定一个目录，当目录中创建或者删除文件时，把相应的信息打印出来
 *    Usage : inotify <dir>
    struct inotify_event {
        __s32           wd;             被监视目标的 watch 描述符
        __u32           mask;           事件掩码
        __u32           cookie;         cookie to synchronize two events
        __u32           len;            name字符串的长度
        char            name[0];        被监视目标的路径名
    };
 */
int main(int argc, char *argv[])
{
    int fd;
    int result;
    char event_buf[512];
    int event_pos = 0;
    struct inotify_event *event;
    struct inotify_event *pos_event;
 
    if(2 != argc) {
        printf("Usage : %s <dir>\n", argv[0]);
        return -1;
    }
 
    /* 1.初始化一个inotify的实例，获得一个该实例的文件描述符 */
    fd = inotify_init();
    if (fd == -1) {
        printf("inotify init error: %s", strerror(errno));
        return -1;
    }
 
    /* 2.添加一个用于监视的目录: 监视该目录中文件的添加和移除修改 */
    result = inotify_add_watch(fd, argv[1], IN_DELETE | IN_CREATE | IN_MODIFY);
    if(-1 == result) {
        printf("inotify_add_watch error:%s\n", strerror(errno));
        return -1;
    }
 
    /* 不停的监视当前目录中是否有添加或者删除文件 */
    while(1) {
        /* 读取inotify的监视事件的信息 */
        memset(event_buf, 0, sizeof(event_buf));
        pos_event = (struct inotify_event *)event_buf;

        /* 阻塞读取 */
        result = read(fd, event_buf, sizeof(event_buf));
        if (result < (int)sizeof(struct inotify_event)) {
            printf("could not read event: %s\n",strerror(errno));
            return -1;
        }

        /* 将获得的inotify信息打印出来 */
        while (result >= (int)sizeof(struct inotify_event)) {
            event = pos_event;
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    printf("create : file is %s\n", event->name);
                } else if (event->mask & IN_DELETE) {
                    printf("delete : file is %s\n", event->name);
                } else if (event->mask & IN_MODIFY) {
                    printf("modify : file is %s\n", event->name);
                }
            }
            
            /* 更新位置信息,以便获得下一个 inotify_event 对象的首地址 */
            pos_event++;
            result -= (int)sizeof(struct inotify_event);
        }
    }
 
    /* 关闭这个文件描述符 */
    close(fd);
 
    return 0;
}
/*
# ./inotify_test /mytest/ &
# touch a.txt 
create : file is a.txt
# cp /etc/profile ./
create : file is profile
modify : file is profile
# 
t# echo hello > a.txt 
modify : file is a.txt
modify : file is a.txt
# 
# echo good >> a.txt      
modify : file is a.txt
# 
*/
