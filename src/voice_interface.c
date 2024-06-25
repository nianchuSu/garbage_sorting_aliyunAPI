#include <stdio.h>
#include <pthread.h>

#include "voice_interface.h"
#include "msg_queue.h"
#include "uarttool.h" // 串口相关

/**
 * 语音线程模块节点API
 */
static int serial_fd = -1; // 全局变量,便于内部函数使用
/**
 * 串口初始化
 */
static int voice_init(void) // 分别实现模块的各个API
{
    serial_fd = uart_Open(URAT, BAUD); // 串口打开
    printf("语音线程初始化...%s | %s | %d | serial_fd:%d\n",
           __FILE__, __func__, __LINE__, serial_fd); // 调试信息

    return serial_fd; // 返回串口标识符
}

/**
 * 释放串口
 */
static void voice_final(void) // 各个回调函数的实现
{
    if (-1 != serial_fd)
    {
        close(serial_fd); // 关闭串口
        serial_fd = -1;
    }
}

/**
 * 语音模块接收指令
 */
static void *voice_get(void *arg)
{
    printf("voice_get is successfully.\n"); // 调试信息
    // unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0x55, 0xAA}; // 初始化指令集
    unsigned char buffer[6] = {0}; // 初始化指令集
    int len = 0;
    mqd_t mqd = -1;                // 消息队列
    ctrl_info_t *ctrl_info = NULL; // 承接参数变量的定义
    if (-1 == serial_fd)           // 判断串口是否正常运行
    {
        serial_fd = voice_init(); // 失败重新初始化
        if (-1 == serial_fd)      // 还失败就线程退出
        {
            printf("%s | %s | %d | serial_fd:%d\n",
                   __FILE__, __func__, __LINE__, serial_fd); // 调试信息
            pthread_exit(0);                                 // 没有串口直接线程退出
        }
    }
    printf("%s | %s | %d \n", __FILE__, __func__, __LINE__);
    /**
     * 获得参数的mqd及其模块地址
     */
    if (NULL != arg)
        ctrl_info = (ctrl_info_t *)arg; // 接收参数

    mqd = ctrl_info->mqd; // 队列标识符
    if ((mqd_t)-1 == mqd) // 判断消息队列
        pthread_exit(0);  // 没有就线程退出

    pthread_detach(pthread_self());                           // 线程分离
    while (1)                                                 // 死循环读取语音模块的接收
    {                                                         // 这里不能计算实际长度,strlen() 遇到0x00会直接返回
        len = uart_Getstr(serial_fd, buffer, sizeof(buffer)); // 读取串口中的信息存入buffer中   // 通过语音模块向串口写入指令
        printf("语音线程待命中...%s | %s | %d | mqd_fd:%d | len:%d buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
               __FILE__, __func__, __LINE__, mqd, len, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
        if (len > 0) // 判断是否读取成功
        {
            /**
             * 读取成功判断buffer内的信息,根据信息做出相应的动作
             */
            if (buffer[0] == 0xAA && buffer[1] == 0x55 && buffer[4] == 0x55 && buffer[5] == 0xAA) // 判断是否有效数据
            {
                send_message(mqd, (void *)buffer, len); // 往消息队列写入信息
                printf("语音发送模块...%s | %s | %d | mqd:%d buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                       __FILE__, __func__, __LINE__, mqd, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            }
            memset(buffer, 0, sizeof(buffer)); // 清空buffer
        }
    }
}

/**
 * 语音模块发送指令/或者根据接收到的指令语音播报
 */
static void *voice_set(void *arg) // 语音播报
{
    pthread_detach(pthread_self());
    unsigned char *buffer = (unsigned char *)arg;

    if (-1 == serial_fd)
    {
        serial_fd = voice_init();
        if (-1 == serial_fd)
        {
            pthread_exit(0);
        }
    }
    if (NULL != buffer)
    {
        printf("向语音模块发送数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
               __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

        uart_Puts(serial_fd, buffer, 6); // 发送给语音模块
    }

    pthread_exit(0);
}

/**
 * 创建节点并赋值
 */
struct control voice_control = // 链表节点赋值(结构体赋值)
    {
        .control_name = "voice", //
        .init = voice_init,      // 绑定回调函数?
        .final = voice_final,
        .get = voice_get,
        .set = voice_set,
        .next = NULL};

/**
 * 使用头插法实现插入链表
 */
struct control *add_voice_to_ctrl_list(struct control *phead)
{

    // return add_interface_to_ctrl_list(phead, voice_control);
    return add_interface_to_ctrl_list(phead, &voice_control);
}