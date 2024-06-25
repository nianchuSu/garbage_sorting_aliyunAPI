#include <errno.h>  // 类型错误头文件

#include "msg_queue.h"
/*
消息队列API
*/
mqd_t msg_queue_create(void)  // 初始化消息队列
{
    mqd_t mqd = -1;
    struct mq_attr attr;
    attr.mq_flags = 0;  // 不阻塞方式
    attr.mq_maxmsg = 10;    // 最大消息条数
    attr.mq_msgsize = SIZE; // 每条消息的最大字节       // 接收信息的存储区,必须大于等于每一条数据的最大字节数  只和
    attr.mq_curmsgs = 0;    // 等待读取的消息数量
    
    // 可读可写可执行的方式创建队列
    mqd = mq_open(QUEQUE_NAME, O_CREAT | O_RDWR, 0666, &attr);    // 打开/创建消息队列
    
    printf("%s | %s | %d | mqd:%d\n",__FILE__, __func__, __LINE__, mqd);

    return mqd; // 成功就返回正确的mqd
}

int send_message(mqd_t mqd, void *msg, int msg_len)  // 向消息队列发送消息
{
    int byte_send = -1;     // 发送之后的返回值
    byte_send = mq_send(mqd, (char *)msg, msg_len, 0);  // 发送
    return byte_send;
}

mqd_t msg_queue_final(mqd_t mqd)
{
    if (-1 != mqd)
    {
        mq_close(mqd);  // 关闭队列
        mq_unlink(QUEQUE_NAME); // 删除队列
    }
    mqd = -1;
    return mqd;
}