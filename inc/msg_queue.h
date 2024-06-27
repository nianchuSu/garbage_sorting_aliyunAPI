#ifndef __MSG_QUEUE__H
#define __MSG_QUEUE__H

#include <mqueue.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "global.h"

#define QUEQUE_NAME "/mq_queue" // 宏定义队列的名称


mqd_t msg_queue_create(void);   // 初始化队列
mqd_t msg_queue_final(mqd_t mqd);   // 关闭及其删除队列
int send_message(mqd_t mqd, void *msg, int msg_len);    // 向队列发送消息

#endif