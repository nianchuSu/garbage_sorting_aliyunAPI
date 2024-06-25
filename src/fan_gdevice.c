#include "gdevice.h"
#include "fan_gdevice.h"

struct gdevice fan_gdev = {
    .dev_name = "fan",    // 名称,风扇
    .key = 0x43,    // 判断数据
    .gpio_pin = 7,  // 引脚
    .gpio_mode = OUTPUT,    // 输出模式
    .gpio_status = HIGH,     // 电平状态     LOW低   HIGH高     控制的是继电器
    .check_face_status = 0, // 是否人脸识别
    .voice_set_status = 0   // 是否回复语音播报
};

/**
 * 插入设备链表
*/
struct gdevice *add_fan_to_gdevice_list( struct gdevice *pdevhead)
{
    return add_gdevice_to_gdevice_list(pdevhead, &fan_gdev);
};
