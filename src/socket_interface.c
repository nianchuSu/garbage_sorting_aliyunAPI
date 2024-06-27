#include <stdio.h>
#include <pthread.h>
#include <netinet/tcp.h>

#include "socket_interface.h"
#include "socket.h"
#include "msg_queue.h"
#include "global.h"
#include "control.h"

/**
 * 网络线程模块节点API
 */
static int socket_fd = -1;
/**
 * 初始化
 */
static int tcpsocket_init(void) // 分别实现模块的各个API
{
    printf("网络线程\n");
    socket_fd = net_init(IPADDR, PORT); // 初始化
    return socket_fd;
}

/**
 * 关闭
 */
static void tcpsocket_final(void) // 各个回调函数的实现
{
    close(socket_fd); // 关闭
    socket_fd = -1;
}

/**
 * 网络模块接收指令
 */
static void *tcpsocket_get(void *arg)
{
    int cloent_fd = -1;             // 客户端句柄
    struct sockaddr_in cloent_addr; // 服务端属性
    char readbuf[6];             // 存储区
    int ret = -1;
    mqd_t mqd = -1;                // 消息队列
    ctrl_info_t *ctrl_info = NULL; // 承接参数变量的定义

    int keepalive = 1; // 开启TCP KeepAlive功能
    int keepidle = 12; // tcp_keepalive_time 180s内没收到数据开始发送心跳包
    int keepcnt = 3;   // tcp_keepalive_probes 每次发送心跳包的时间间隔,单位秒
    int keepintvl = 3; // tcp_keepalive_intvl 每10s发送一次心跳包

    pthread_detach(pthread_self()); // 线程分离
    printf("%s %s %d--socket_fd:%d\n", __FILE__, __func__, __LINE__, socket_fd);
    if (-1 == socket_fd)
    {
        socket_fd = tcpsocket_init(); // 重新初始化
        if (-1 == socket_fd)          // 还失败就退出程序
        {
            printf("%s %s %d--socket_fd:%d\n", __FILE__, __func__, __LINE__, socket_fd);
            perror("net_init");
            pthread_exit(0);
        }
    }

    if (NULL != arg)
        ctrl_info = (ctrl_info_t *)arg; // 接收参数
    if (NULL != ctrl_info)
        mqd = ctrl_info->mqd;               // 队列标识符
    if ((mqd_t)-1 == mqd) // 判断消息队列
        pthread_exit(0);  // 没有就线程退出

    printf("%s|%s|%d|socket_fd:%d,网络服务开启成功.\n", __FILE__, __func__, __LINE__, socket_fd);
    memset(&cloent_addr, 0, sizeof(struct sockaddr_in)); // Data clearing(数据清空)
    int len = sizeof(struct sockaddr_in);                // Length is pointer(长度是指针)
    while (1)
    {
        cloent_fd = accept(socket_fd, (struct sockaddr *)&cloent_addr, &len); // Return the request client connection handle that is needed for communication(返回的请求客户端连接句柄,通信中需要用的这个句柄)
        printf("网络监测中...%s %s %d cloent_fd:%d\n", __FILE__, __func__, __LINE__, cloent_fd);

        printf("客户端接入...%s %s %d--ip:%s port:%d\n", __FILE__, __func__, __LINE__, inet_ntoa(cloent_addr.sin_addr), ntohs(cloent_addr.sin_port));
        if (cloent_fd == -1)
        {
            perror("accept");
            continue; // 跳过本次
        }

        ret = setsockopt(cloent_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
        if (-1 == ret)
        {
            perror("setsockopt");
            break;
        }
        ret = setsockopt(cloent_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
        if (-1 == ret)
        {
            perror("setsockopt");
            break;
        }
        ret = setsockopt(cloent_fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt));
        if (-1 == ret)
        {
            perror("setsockopt");
            break;
        }
        ret = setsockopt(cloent_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl));
        if (-1 == ret)
        {
            perror("setsockopt");
            break;
        }
        printf("客户端信息...IP:%s,port:%d\n", inet_ntoa(cloent_addr.sin_addr), ntohs(cloent_addr.sin_port)); // 打印客户端的信息IP和端口

        while (1)
        {
            printf("网络线程执行中...%s %s %d cloent_fd:%d\n", __FILE__, __func__, __LINE__, cloent_fd);
            memset(readbuf, 0, sizeof(readbuf));
            ret = recv(cloent_fd, readbuf, SIZE, 0); // 这里会阻塞
            printf("BUF数据...%s %s %d-- ret:%d readbuf[0]:0x%X,readbuf[1]:0x%X,readbuf[2]:0x%X,readbuf[3]:0x%X,readbuf[4]:0x%X,readbuf[5]:0x%X\n",
                     __FILE__, __func__, __LINE__, ret, readbuf[0], readbuf[1], readbuf[2], readbuf[3], readbuf[4], readbuf[5]);
            if (ret > 0)
            {
                printf("buf数据...%s | %s | %d | mqd:%d | len:%d\n", __FILE__, __func__, __LINE__, mqd, len);
                /**
                 * 读取成功判断buffer内的信息,根据信息做出相应的动作
                 */
                if (readbuf[0] == 0xAA && readbuf[1] == 0x55 && readbuf[4] == 0x55 && readbuf[5] == 0xAA) // 判断是否有效数据
                {
                    send_message(mqd, (void *)readbuf, 6); // 往消息队列写入信息
                    printf("buf实际有效数据...%s | %s | %d | mqd:%d readbuf[0]:0x%X,readbuf[1]:0x%X,readbuf[2]:0x%X,readbuf[3]:0x%X,readbuf[4]:0x%X,readbuf[5]:0x%X\n",
                             __FILE__, __func__, __LINE__, mqd, readbuf[0], readbuf[1], readbuf[2], readbuf[3], readbuf[4], readbuf[5]);
                }
            }
            else
            {
                break;
            }
        }
    }
    pthread_exit(0);
}

/**
 * 网络模块发送指令/或者根据接收到的指令网络播报
 */
static void *tcpsocket_set(void *arg)
{
}

/**
 * 创建节点并赋值
 */
struct control socket_control = // 链表节点赋值(结构体赋值)
    {
        .control_name = "socket", //
        .init = tcpsocket_init,   // 绑定回调函数?
        .final = tcpsocket_final,
        .get = tcpsocket_get,
        .set = tcpsocket_set,
        .next = NULL};

/**
 * 使用头插法实现插入链表
 */
struct control *add_tcpsocket_to_ctrl_list(struct control *phead)
{

    // return add_interface_to_ctrl_list(phead, socket_control);
    return add_interface_to_ctrl_list(phead, &socket_control);
}