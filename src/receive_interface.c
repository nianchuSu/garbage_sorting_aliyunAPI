#include <stdio.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <wiringPi.h>

#include "receive_interface.h"
#include "socket.h"
#include "msg_queue.h"
#include "global.h"
#include "control.h"
#include "myoled.h" // oled
#include "face.h"   // 人脸识别
#include "uarttool.h"
#include "gdevice.h"         // 设备模块
#include "lrled_gdevice.h"   // 客厅灯
#include "roomled_gdevice.h" // 房间灯
#include "fan_gdevice.h"     // 风扇
#include "beep_gdevice.h"    // 蜂鸣器
#include "lock_gdevice.h"    // 门锁

typedef struct
{
    int msg_len;
    unsigned char *buffer;
    ctrl_info_t *ctrl_info;
} recv_msg_t;

static int oled_fd = -1;                // oled句柄
static struct gdevice *pdevhead = NULL; // 定义并初始化设备链表
/**
 * 语音接收线程模块节点API
 */
static int receive_fd = -1;
/**
 * 初始化
 */
static int receive_init(void) // 分别实现模块的各个API
{
    printf("语音接收线程\n");
    oled_fd = myoled_init();                          // 初始化oled
    py_init();                                        // 初始化人脸识别接口python环境
    pdevhead = add_lrled_to_gdevice_list(pdevhead);   // 客厅灯添加至设备链表
    pdevhead = add_roomled_to_gdevice_list(pdevhead); // 房间灯添加至设备链表
    pdevhead = add_fan_to_gdevice_list(pdevhead);     // 风扇添加至设备链表
    pdevhead = add_beep_to_gdevice_list(pdevhead);    // 蜂鸣器添加至设备链表
    pdevhead = add_lock_to_gdevice_list(pdevhead);    // 门锁添加至设备链表
    return 0;
}

/**
 * 关闭
 */
static void receive_final(void) // 各个回调函数的实现
{
    py_finish(); // 释放python环境
    if (-1 != oled_fd)
    {
        close(oled_fd); // 关闭oled
        oled_fd = -1;
    }
}

static void *handld_device(void *arg) // 设备处理函数
{
    recv_msg_t *recv_msg = NULL;    // 承接参数
    struct gdevice *cur_dev = NULL; // 承接设备的空指针
    char success_or_failed[10] = "success";
    int ret = -1;
    pthread_t tid = -1;
    int oled_flag = 0;   // oled显示标志位
    double face_ret = 0; // 人脸识别判断

    pthread_detach(pthread_self()); // 线程分离

    if (NULL != arg)
    {
        recv_msg = (recv_msg_t *)arg;
        printf("设备指令线程线程...%s | %s | %d | read_len:%d buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
               __FILE__, __func__, __LINE__, recv_msg->msg_len,
               recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
    }
    /**
     * 数据指令的相应动作(处理)
     */
    if (NULL != recv_msg && NULL != recv_msg->buffer)
    {                                                                // 就获得要操作的设备了,
        cur_dev = find_device_by_key(pdevhead, recv_msg->buffer[2]); // 查找设备
        if (NULL == cur_dev)                                         // 没找到就退出线程
        {
            printf("没有找到这个设备...%s | %s | %d | read_len:%d recv_msg->buffer[2]%X\n",
                   __FILE__, __func__, __LINE__, recv_msg->msg_len, recv_msg->buffer[2]);
            pthread_exit(0);
        }
    }
    printf("找到该设备设备...%s | %s | %d | read_len:%d 设备名称:%s\n",
           __FILE__, __func__, __LINE__, recv_msg->msg_len, cur_dev->dev_name);
    printf("cur_dev->voice_set_status:%d\n", cur_dev->voice_set_status);

    cur_dev->gpio_status = recv_msg->buffer[3] == 0 ? LOW : HIGH; // 判断目前状态并赋值
    if (1 == cur_dev->check_face_status)                          // 需要摄像头
    {
        face_ret = par_retfunc(); // 调用拍照并比对,返回相似度
        printf("face_ret%lf\n", face_ret);
        if (face_ret > 0.5)
        {
            recv_msg->buffer[3] = 0x2E;
            ret = set_gpio_gdevice_status(cur_dev); // 开门成功
            printf("buffer数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                   __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
        }
        else
        {
            recv_msg->buffer[3] = 0x2F;
            ret = -1; // 开门失败
            printf("buffer数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                   __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
        }
    }
    else
    {
        // 三目运算     操作对应的设备
        // cur_dev->gpio_status = recv_msg->buffer[3] == 0 ? LOW : HIGH; // 判断目前状态并赋值
        ret = set_gpio_gdevice_status(cur_dev); // 操作对应设备
    }

    if (1 == cur_dev->voice_set_status) // 语音播报
    {
        if (NULL != recv_msg && NULL != recv_msg->ctrl_info && NULL != recv_msg->ctrl_info->ctrl_phead)
        // if (NULL != recv_msg->ctrl_info->ctrl_phead)
        {
            struct control *pcontrol = recv_msg->ctrl_info->ctrl_phead;
            while (NULL != pcontrol) // 遍历链表
            {
                if (strstr(pcontrol->control_name, "voice"))
                {
                    if (0x45 == recv_msg->buffer[2] && 0x00 == recv_msg->buffer[3])
                    {
                        oled_flag = 1; // 改变oled显示标志位
                        printf("buffer原数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                               __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
                        recv_msg->buffer[2] = 0x2D;
                        recv_msg->buffer[3] = 0x2D; // 语音模块的接收数据的问题
                        printf("buffer数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                               __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
                    }
                    else if (0x44 == recv_msg->buffer[2] && 0x2E == recv_msg->buffer[3])
                    { // 开门成功
                        recv_msg->buffer[2] = 0x2E;
                        printf("buffer数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                               __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
                    }
                    else if (0x44 == recv_msg->buffer[2] && 0x2F == recv_msg->buffer[3])
                    { // 开门失败
                        recv_msg->buffer[2] = 0x2F;
                        printf("buffer数据...%d, buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
                               __LINE__, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);
                    }
                    // 想要语音多播报几次,就在语音模块哪里设置
                    pthread_create(&tid, NULL, pcontrol->set, (void *)recv_msg->buffer);
                    usleep(100); // 延时,防止数据发送出错,导致语音模块不播报
                    recv_msg->buffer[2] = 0x00;
                    recv_msg->buffer[3] = 0x00; // 手动重置到进入这里的原始数据数据,下面的对应的需要判断
                    break;
                }
                pcontrol = pcontrol->next;
            }
        }
    }

    if (-1 == ret)
    {
        memset(success_or_failed, 0, sizeof(success_or_failed));
        strncpy(success_or_failed, "failed", 6);
    }

    // OLED显示
    char oled_msg[512];
    memset(oled_msg, 0, sizeof(oled_msg)); // 清空内存初始化空间
    char *change_status = cur_dev->gpio_status == LOW ? "Open" : "Close";
    // sprintf(oled_msg, "%s %s %s.\n", change_status, cur_dev->dev_name, success_or_failed);
    if (1 == oled_flag)
    {
        // memset(oled_msg, 0, sizeof(oled_msg)); // 清空内存初始化空间
        sprintf(oled_msg, "%s %s.\n", change_status, "Trigger danger");
    }
    else if (2 == oled_flag)
    { // 显示开门是否成功
    }
    else
    {
        // memset(oled_msg, 0, sizeof(oled_msg)); // 清空内存初始化空间
        // char *change_status = cur_dev->gpio_status == LOW ? "Open" : "Close";
        sprintf(oled_msg, "%s %s %s.\n", change_status, cur_dev->dev_name, success_or_failed);
    }
    myoled_show(oled_msg);

    printf("%s %s %d|cur_dev->check_face_status:%d|ret:%d|face_ret:%.2lf\n", __FILE__, __func__, __LINE__, cur_dev->check_face_status, ret, face_ret);
    // 关门
    if (1 == cur_dev->check_face_status && 0 == ret && face_ret > 0.5)
    {
        sleep(5);
        cur_dev->gpio_status = HIGH; // 变为高电平
        set_gpio_gdevice_status(cur_dev);
        // face_ret = 0;    // 可以不用
    }
    pthread_exit(0);
}

/**
 * 语音接收模块
 */
static void *receive_get(void *arg)
{
    recv_msg_t *recv_msg = NULL; // 承接参数
    char *buffer = NULL;
    struct mq_attr attr;
    ssize_t read_len = -1;
    pthread_t tid = -1;

    if (NULL != arg)
    {
        recv_msg = (recv_msg_t *)malloc(sizeof(recv_msg_t)); // 申请空间
        recv_msg->ctrl_info = (ctrl_info_t *)arg;            // 接收参数
        recv_msg->msg_len = -1;
        recv_msg->buffer = NULL;
    }
    else
    {
        pthread_exit(0);
    }

    if (-1 == mq_getattr(recv_msg->ctrl_info->mqd, &attr)) // 获得消息队列的属性
    {                                                      // 目的是取出消息队列的每条消息的最大字节
        pthread_exit(0);
    }
    // recv_msg->buffer = (unsigned char *)malloc(SIZE);  // 申请空间
    recv_msg->buffer = (unsigned char *)malloc(attr.mq_msgsize); // 申请空间
    memset(recv_msg->buffer, 0, attr.mq_msgsize);                // 初始化内存空间
    buffer = (unsigned char *)malloc(attr.mq_msgsize);           // 申请空间
    memset(buffer, 0, attr.mq_msgsize);                          // 初始化内存空间

    pthread_detach(pthread_self()); // 线程分离
    while (1)
    {
        read_len = mq_receive(recv_msg->ctrl_info->mqd, buffer, attr.mq_msgsize, NULL); // 接收消息队列的数据
                                                                                        // recv_msg->buffer = buffer;  // 指针指向
        printf("语音接收线程...%s | %s | %d | read_len:%ld buffer[0]:0x%X,buffer[1]:0x%X,buffer[2]:0x%X,buffer[3]:0x%X,buffer[4]:0x%X,buffer[5]:0x%X\n",
               __FILE__, __func__, __LINE__, read_len, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

        if (-1 == read_len)
        {
            if (errno == EAGAIN)
            {
                printf("消息队列为空\n");
            }
            else // 还可以有其他情况
            {
                break;
            }
        }
        else if (buffer[0] == 0xAA && buffer[1] == 0x55 && buffer[4] == 0x55 && buffer[5] == 0xAA) // 判断是否有效数据
        {
            recv_msg->msg_len = read_len;
            memcpy(recv_msg->buffer, buffer, read_len);                  // 内存块拷贝
            pthread_create(&tid, NULL, handld_device, (void *)recv_msg); // 创建新的线程,绑定回调函数handld_device
        }
    }

    pthread_exit(0);
}

/**
 * 网络模块发送指令/或者根据接收到的指令网络播报
 */
static void *receive_set(void *arg)
{
}

/**
 * 创建节点并赋值
 */
struct control receive_control = // 链表节点赋值(结构体赋值)
    {
        .control_name = "receive", //
        .init = receive_init,      // 绑定回调函数?
        .final = receive_final,
        .get = receive_get,
        .set = receive_set,
        .next = NULL};

/**
 * 使用头插法实现插入链表
 */
struct control *add_receive_to_ctrl_list(struct control *phead)
{

    return add_interface_to_ctrl_list(phead, &receive_control);
}