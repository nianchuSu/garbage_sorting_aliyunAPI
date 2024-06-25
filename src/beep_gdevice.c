#include "gdevice.h"
#include "beep_gdevice.h"

struct gdevice beep_gdev = {
    .dev_name = "beep",    // 名称,蜂鸣器
    .key = 0x45,    // 判断数据
    .gpio_pin = 10,  // 引脚
    .gpio_mode = OUTPUT,    // 输出模式
    .gpio_status = HIGH,     // 电平状态     LOW低   HIGH高     控制的是继电器
    .check_face_status = 0, // 是否人脸识别
    .voice_set_status = 1   // 是否回复语音播报
};

/**
 * 插入设备链表
*/
struct gdevice *add_beep_to_gdevice_list( struct gdevice *pdevhead)
{
    return add_gdevice_to_gdevice_list(pdevhead, &beep_gdev);
};
