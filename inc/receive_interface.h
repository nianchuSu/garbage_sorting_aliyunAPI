#ifndef __RECEIVE_INTERFACE__H
#define __RECEIVE_INTERFACE__H

#include "control.h"
#include "global.h"     // 参数头文件

struct control *add_receive_to_ctrl_list(struct control *phead);

#endif