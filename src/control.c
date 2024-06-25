#include <stdio.h>

#include "control.h"

#if 1
struct control *add_interface_to_ctrl_list(struct control *phead, struct control *control_interface)
{

    if (NULL == phead)
    {
        phead = control_interface; // 链表指向
    }
    else
    {
        control_interface->next = phead; // 内部使用无需传参直接使用即可
        phead = control_interface;
    }
    return phead;
}
#endif

#if 0
struct control *add_interface_to_ctrl_list(struct control *phead, struct control control_interface)
{

    if (NULL == phead)
    {
        phead = &control_interface; // 链表指向
    }
    else
    {
        control_interface.next = phead; // 内部使用无需传参直接使用即可
        phead = &control_interface;
    }
    return phead;
}
#endif