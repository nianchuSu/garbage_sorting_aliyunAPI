#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "voice_interface.h"  // 语音模块
#include "socket_interface.h" // 网络模块
#include "smoke_interface.h" // 烟雾模块
#include "receive_interface.h"  // 语音接收线程并语音播报动作
#include "msg_queue.h"        // 消息队列
#include "socket.h"           // 消息队列
#include "control.h"          // 多线程模块
#include "global.h"           // 参数头文件


int main(int argc, char const *argv[])
{
    int node_num = 0; // 计算多线程的模块数量
    pthread_t thread_id;
    struct control *control_phead = NULL;                   // 创建多线程链表
    struct control *pointer = NULL;                         // 中间变量指针
    ctrl_info_t *ctrl_info = NULL;                          // 定义参数变量指针
    ctrl_info = (ctrl_info_t *)malloc(sizeof(ctrl_info_t)); // 开辟内存空间
    ctrl_info->mqd = -1;                                    // 初始化参数
    ctrl_info->ctrl_phead = NULL;                           // 初始化参数

    printf("parameter is successfully.\n");

    if (-1 == wiringPiSetup()) // 初始化GPIO(香橙派)
    {
        perror("wiringPiSetup error\n");    // 失败直接退出
        return -1;
    }

    ctrl_info->mqd = msg_queue_create(); // 创建消息队列,获得队列标识符
    if (-1 == ctrl_info->mqd)
    {
        perror("ctrl_info->mqd");
        return -1;
    }
    printf("mqd is successfully.\n");
    /**
     * 调用多线程模块的add函数新增如链表
     */
    ctrl_info->ctrl_phead = add_voice_to_ctrl_list(ctrl_info->ctrl_phead);     // 语音模块接入链表
    ctrl_info->ctrl_phead = add_tcpsocket_to_ctrl_list(ctrl_info->ctrl_phead); // 网络模块接入链表
    ctrl_info->ctrl_phead = add_smoke_to_ctrl_list(ctrl_info->ctrl_phead); // 烟雾模块接入链表
    ctrl_info->ctrl_phead = add_receive_to_ctrl_list(ctrl_info->ctrl_phead); // 语音接收模块接入链表
    /**
     * 这还可以有其他模块接入链表,参考语音模块voice_interface.c代码
     */
    // 网络模块线程接入
    // 火灾检测
    // ...其他多线程等
    printf("ctrl_phead is successfully.\n");

    pointer = ctrl_info->ctrl_phead; // 指向链表头
    while (NULL != pointer)          // 初始化各线程模块
    {
        if (NULL != pointer->init)
        {
            pointer->init();
        }
        pointer = pointer->next;
        node_num++; // 计算模块数量
    }
    printf("init is successfully.\n");

    pthread_t *tid = malloc(sizeof(pthread_t) * node_num); // 有多少各模块就有多少个线程PID
    printf("PID is successfully.\n");

    pointer = ctrl_info->ctrl_phead; // 重新指向链表头
    for (int i = 0; i < node_num; i++)
    { // 绑定线程回调函数
        if (NULL != pointer->get)
            pthread_create(&tid[i], NULL, (void *)pointer->get, (void *)ctrl_info);
        pointer = pointer->next;
    }
    printf("get is successfully.\n");

    /**
     * 线程等待
     */
    for (int i = 0; i < node_num; i++)
    {
        pthread_join(tid[i], NULL);
    }
    printf("pthread_join is successfully.\n");

    while (NULL != pointer) // 释放各线程模块
    {
        if (NULL != pointer->final)
        {
            pointer->final();
        }
        pointer = pointer->next;
    }
    printf("final is successfully.\n");

    msg_queue_final(ctrl_info->mqd); // 队列的释放及其关闭
    if (NULL != ctrl_info)
    {
        free(ctrl_info);    // 释放参数
    }
    if (NULL != tid)
    {
        free(tid);  // 释放线程pid(线程池?)
    }
    
    return 0;
}
