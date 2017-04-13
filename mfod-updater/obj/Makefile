CROSS_COMPILE = /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib

CFLAGS=-c -g -std=gnu99 -O0 -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9 \
         -mfpu=vfpv3 -mfloat-abi=softfp
LDFLAGS =

DIR_ROOT = /home/kjj/workspace/mfod-updater/
DIR_ROOTFS = /opt/imx6-resource/rootfs/mfod-mmcblk0p2
DIR_OUT = $(DIR_ROOT)out/
DIR_OBJ = $(DIR_ROOT)obj/
VPATH = src

LIBS = -lrt -lpthread -ldirectfb -lfusion -ldirect -ldl -lm -lc -lz \
	   -lipu -ljpeg -lmorpho -lcoordtr -lgeomag

LOCAL_LDFLAGS += -L/opt/imx6-resource/rootfs/mfod-mmcblk0p2/usr/lib \
				 -L$(DIR_ROOT)lib/

DIR_INC += -I/opt/imx6-resource/rootfs/mfod-mmcblk0p2/usr/include/ \
		   -I/opt/imx6-resource/rootfs/mfod-mmcblk0p2/usr/include/directfb \
		   -I$(DIR_ROOT)inc/

TARGET = mfod-updater

all:
	$(CC) *.o -g -o $(TARGET) $(LIBS) $(DIR_INC) $(LOCAL_LDFLAGS)

link_objects:
	$(CC) *.o -g -o $(TARGET) $(LIBS) $(DIR_INC) $(LOCAL_LDFLAGS)

clean:
	rm  *.o
	rm  $(TARGET)
