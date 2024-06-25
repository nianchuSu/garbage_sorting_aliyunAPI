#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "oled.h"
#include "font.h"

#include "myoled.h"

#define IIC "/dev/i2c-3"

static struct display_info disp;

int myoled_show(void *arg)
{
	oled_clear(&disp);
	unsigned char *buf = (unsigned char *)arg; // 强转数据类型
	if (NULL == buf)	// 没有数据就结束
	{
		return -1;
	}
	
	oled_putstrto(&disp, 0, 4, buf);
	
	disp.font = font2;
	oled_send_buffer(&disp);
	return 0;
}

int myoled_init(void)
{
	int i;
	disp.address = OLED_I2C_ADDR;
	disp.font = font2;

	i = oled_open(&disp, IIC);
	i = oled_init(&disp);
	oled_clear(&disp); // 清屏

	return i;
}