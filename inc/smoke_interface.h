#ifndef __SMOKE_INTERFACE__H
#define __SMOKE_INTERFACE__H

#include "control.h"

#define SMOKE_PIN 9     // 针脚
#define SMOKE_MODE INPUT    // 输入输出

struct control *add_smoke_to_ctrl_list(struct control *phead);  // 接入线程链表

#endif