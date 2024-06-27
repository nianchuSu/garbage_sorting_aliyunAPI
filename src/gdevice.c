#include <stdio.h>

#include "gdevice.h"
#include "global.h"

#if 0
/**
 * 添加设备节点链表方法
 */
struct gdevice *add_gdevice_to_gdevice_list(struct gdevice *pdevhead, struct gdevice *pdevice_interface)
{   // 头插法
    if (NULL == pdevhead)
    {
        pdevhead = pdevice_interface;
    }
    else
    {
        pdevice_interface->next = pdevhead;    // 新增设备节点替换头部位置
        pdevhead = pdevice_interface;      // 头部重新指向新增设备
    }
    
}
#endif

/**
 * 查找设备
*/
struct gdevice *find_device_by_key(struct gdevice *pdevhead, int key) // 找到对应的设备
{
    struct gdevice *p = NULL;

    if (NULL == pdevhead)
    {
        return NULL;    // 链表为空也返回NULL
    }

    p = pdevhead;

    while (NULL != p)
    {
        if (p->key == key)  // 找到就返回指针
        {
            return p;
        }
        p = p->next;
    }
    
    return NULL;    // 没找到就返回NULL
}

/**
 * 设置设备的状态
*/
int set_gpio_gdevice_status(struct gdevice *pdev)   // 参数就是单个设备的指针
{
    if (NULL == pdev)
    {
        return -1;
    }
    
    if (-1 != pdev->gpio_pin)
    {
        if (-1 != pdev->gpio_mode)
        {
            pinMode(pdev->gpio_pin, pdev->gpio_mode);   // 设置设备输入输出
        }
        if (-1 != pdev->gpio_status)    // 设备位输出模式时,引脚电平的高低
        {
            digitalWrite(pdev->gpio_pin, pdev->gpio_status);
        }        
    }
    return 0;
}