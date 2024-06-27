#ifndef __GDEVICE__H
#define __GDEVICE__H

#include <stdio.h>
#include <wiringPi.h>

struct gdevice  // 设备模块API
{
    char dev_name[36];  // 设备名称
    int key;    // 控制设备的指令?
    int gpio_pin;   // 设备的IO口
    int gpio_mode;  // IO口的模式
    int gpio_status;    // IO口的电平设置
    int check_face_status;  // 是否调用摄像头拍照人脸检测
    int voice_set_status;   // 是否调用语音播放
    struct gdevice *next;   // 设备节点指向下一个设备的指针
};

// struct gdevice *add_gdevice_to_gdevice_list(struct gdevice *phead, struct gdevice *pdevice_interface); // 添加设备节点链表方法
struct gdevice *find_device_by_key(struct gdevice *phead, int key); // 找到对应的设备
int set_gpio_gdevice_status(struct gdevice *pdev);

#endif