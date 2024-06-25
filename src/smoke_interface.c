/**
 * 烟雾检测模块
*/
#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>

#include "smoke_interface.h"
#include "msg_queue.h"
#include "global.h"
#include "control.h"



/**
 * 烟雾报警线程模块节点API
 */
/**
 * 初始化
 */
static int smoke_init(void) // 分别实现模块的各个API
{
    printf("%s %s %d\n", __FILE__, __func__, __LINE__);
    pinMode(SMOKE_PIN, SMOKE_MODE);     // 设置针脚的输入输出模式
    return 0;
}

/**
 * 关闭
 */
static void smoke_final(void) // 各个回调函数的实现
{
}

/**
 * 烟雾模块接收指令
 */
static void *smoke_get(void *arg)
{
    printf("烟雾线程.\n");
    int status = HIGH; // 状态码?
    int switch_status = 0;  // 判断状态码?
    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0x55, 0xAA};
    ssize_t byte_send = -1; // 消息队列相关
    mqd_t mqd = -1;     // 消息队列标识符
    ctrl_info_t *ctrl_info = NULL; // 承接参数变量的定义

    pthread_detach(pthread_self()); // 线程分离

    if (NULL != arg)
        ctrl_info = (ctrl_info_t *)arg; // 接收参数
    if (NULL != ctrl_info)
        mqd = ctrl_info->mqd;               // 队列标识符
    if ((mqd_t)-1 == mqd) // 判断消息队列
        pthread_exit(0);  // 没有就线程退出
    printf("%s %s %d mqd:%d\n", __FILE__, __func__, __LINE__, mqd);
    while (1)   // 循环监测周围环境是否有火情及其煤气泄漏
    {

        printf("烟雾报警正在监测中...%s %s %d mqd:%d buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                 __FILE__, __func__, __LINE__, mqd, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

        status = digitalRead(SMOKE_PIN);    // 读取烟雾报警器的状态
        if (LOW == status)  // 低电平触发报警
        {
            buffer[2] = 0x45;   // 改变数据
            buffer[3] = 0x00;
            switch_status = 1;  // 烟雾报警触发改变其状态

            byte_send = mq_send(mqd, buffer, 6, 0);    // 发送指令(发送数据)
            if (-1 == byte_send)
            {
                continue;   // 失败跳过本次循环
            }

            printf("监测到险情,清尽快撤离现场...%s %s %d byte_send:%ld  buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                    __FILE__, __func__, __LINE__, byte_send, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

            byte_send = -1;
        }
        else if (HIGH == status && 1 == switch_status)  // 报警结束后
        {
            buffer[2] = 0x45;   // 改变数据
            buffer[3] = 0x01;
            switch_status = 0;
            byte_send = mq_send(mqd, buffer, 6, 0);    // 发送指令(发送数据)
            if (-1 == byte_send)
            {
                continue;   // 失败跳过本次循环
            }
            // printf("险情已结束\n");
            printf("险情已结束...%s %s %d byte_send:%ld  buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                    __FILE__, __func__, __LINE__, byte_send, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            buffer[2] = 0x00;
            // buffer[3] = 0x00;
            byte_send = -1;
        }
        sleep(5);   // 延时,还有切换线程的功能(作用)
    }
    
    pthread_exit(0);
}

/**
 * 烟雾模块发送指令/或者根据接收到的指令网络播报
 */
static void *smoke_set(void *arg)
{
}

/**
 * 创建节点并赋值
 */
struct control smoke_control = // 链表节点赋值(结构体赋值)
    {
        .control_name = "smoke", //
        .init = smoke_init,   // 绑定回调函数?
        .final = smoke_final,
        .get = smoke_get,
        .set = smoke_set,
        .next = NULL};

/**
 * 使用头插法实现插入链表
 */
struct control *add_smoke_to_ctrl_list(struct control *phead)
{

    // return add_interface_to_ctrl_list(phead, smoke_control);
    return add_interface_to_ctrl_list(phead, &smoke_control);
}