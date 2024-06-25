#! /bin/bash	
# 第一行执行脚本解释器

cd /home/orangepi/mjpg-streamer/mjpg-streamer-experimental		# 先到执行脚本路径
./start.sh		# 在执行脚本

#需要下载
#mjpg-streamer
#安装包调用摄像头
#安装完成后需要修改配置文件start.sh
#./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www	// 原本内容

#./mjpg_streamer -i "./input_uvc.so -d /dev/video1 -u -f 30" -o "./output_http.so -w ./www"	// 修改后
