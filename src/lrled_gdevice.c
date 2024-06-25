#include "gdevice.h"
#include "lrled_gdevice.h"

struct gdevice lrled_gdev = {
    .dev_name = "lrled",    // 名称,客厅灯
    .key = 0x41,    // 判断数据
    .gpio_pin = 5,  // 引脚
    .gpio_mode = OUTPUT,    // 输出模式
    .gpio_status = HIGH,     // 电平状态     LOW低   HIGH高     控制的是继电器
    .check_face_status = 0, // 是否人脸识别
    .voice_set_status = 0   // 是否回复语音播报
};

/**
 * 插入设备链表
*/
struct gdevice *add_lrled_to_gdevice_list( struct gdevice *pdevhead)
{
    return add_gdevice_to_gdevice_list(pdevhead, &lrled_gdev);
};
