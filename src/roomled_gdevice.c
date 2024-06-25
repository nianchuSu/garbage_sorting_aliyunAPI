#include "gdevice.h"
#include "roomled_gdevice.h"

struct gdevice roomled_gdev = {
    .dev_name = "roomled",    // 名称,房间灯
    .key = 0x42,    // 判断数据
    .gpio_pin = 6,  // 引脚
    .gpio_mode = OUTPUT,    // 输出模式
    .gpio_status = HIGH,     // 电平状态     LOW低   HIGH高     控制的是继电器
    .check_face_status = 0, // 是否人脸识别
    .voice_set_status = 0   // 是否回复语音播报
};

/**
 * 插入设备链表
*/
struct gdevice *add_roomled_to_gdevice_list( struct gdevice *pdevhead)
{
    return add_gdevice_to_gdevice_list(pdevhead, &roomled_gdev);
};
