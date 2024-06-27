#ifndef __SOCKET_INTERFACE__H
#define __SOCKET_INTERFACE__H

#include "control.h"
#include "global.h"     // 参数头文件

struct control *add_tcpsocket_to_ctrl_list(struct control *phead);

#endif