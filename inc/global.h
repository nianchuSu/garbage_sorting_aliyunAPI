#ifndef __GLOBAL__H
#define __GLOBAL__H

#include <mqueue.h>

#define SIZE 108    // buffer存储区大小

/**
 * 参数头文件
*/
typedef struct
{
    mqd_t mqd;  
    struct control *ctrl_phead;
}ctrl_info_t;   // 参数结构体



#endif