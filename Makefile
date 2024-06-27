# CC := aarch64-none-linux-gnu-gcc		# 编译工具
CC := aarch64-linux-gnu-gcc		# 编译工具
SRC := $(shell find src -name "*.c")	# 获取所有的.c文件
# 头文件路径
INC := ./inc \
	./src \
	./3rd/include \
	./3rd/usr/include \
	./3rd/usr/include/python3.10 \
	./3rd/usr/include/aarch64-linux-gnu/python3.10 \
	./3rd/lib/aarch64-linux-gnu \
	./3rd/usr/local/include

OBJ := $(subst src/,obj/,$(SRC:.c=.o))	# 生成.o并且修改存放路径

TARGET=obj/home	# 存放路径	目标

CFLAGS := $(foreach item,$(INC),-I $(item))	# 连接头文件路径	-I
# 库文件路径
LIBS_PATH := ./3rd/usr/lib/python3.10/config-3.10-aarch64-linux-gnu \
	./3rd/lib/aarch64-linux-gnu \
	./3rd/usr/lib/aarch64-linux-gnu \
	./3rd/usr/include \
	./3rd/usr/include/python3.10 \
	./3rd/usr/local/lib
	
LDFLAGS := $(foreach item,$(LIBS_PATH),-L $(item))	# 连接库文件路径	-L

LIBS := -lwiringPi -lpython3.10	-pthread -lexpat -lz -lcrypt	# 链接库
obj/%.o:src/%.c		# 生成.o文件
	mkdir -p obj
	$(CC) -o $@ -c $< $(CFLAGS)
$(TARGET) : $(OBJ)		# 链接所有.o文件
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)
	scp ./obj/home ./src/face.py ./ini/peripheral.ini orangepi@192.168.1.250:/home/orangepi/house

compile : $(TARGET)

clean :		# 强制删除
	rm $(TARGET) obj $(OBJ) -rf

show:	# 打印各项信息
	@echo $(CC)
	@echo $(SRC)
	@echo $(INC)
	@echo $(OBJ)
	@echo $(TARGET)
	@echo $(CFLAGS)
	@echo $(LDFLAGS)
	@echo $(LIBS)
	@echo $(OBJ)
	
#	@echo $()

.PHONY : clean compile show		# 定义伪目标