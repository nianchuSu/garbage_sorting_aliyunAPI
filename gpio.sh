#!/bin/bash

if !which gpio >/dev/null 2>&1; then
	echo "please install wiringOP first"
	exit 1
fi

gpio mode 5 out		# 客厅灯
gpio mode 6 out		# 房间灯
gpio mode 7 out		# 风扇
gpio mode 8 out		# 门锁
gpio mode 10 out	# 蜂鸣器

for i in 5 6 7 8 10
do
	gpio write $i 1
done

for i in 5 6 7 8 10
do
	gpio write $i 0
	sleep 3
	gpio write $i 1
done

gpio mode 9 in	# 烟雾报警器

gpio readall
sleep 5
gpio readall
