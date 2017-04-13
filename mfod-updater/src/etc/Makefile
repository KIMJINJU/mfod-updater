CROSS_COMPILE = /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib

CFLAGS=-c -g -std=gnu99 -O0 -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9 \
         -mfpu=vfpv3 -mfloat-abi=softfp -D _GNU_SOURCE

LDFLAGS =

DIR_ROOT = /home/kjj/workspace/mfod-updater/
DIR_ROOTFS = /opt/imx6-resource/rootfs/mfod-mmcblk0p2
DIR_OUT = $(DIR_ROOT)out/

LIBS = -lrt -lpthread -ldl -lm -lc -lz -liconv\

LOCAL_LDFLAGS += -L/opt/imx6-resource/rootfs/amod/usr/lib \
				 -L$(DIR_ROOT)lib

DIR_INC += -I/opt/imx6-resource/rootfs/amod/usr/include/ \
		   -I/opt/imx6-resource/rootfs/amod/usr/include/directfb \
		   -I/opt/imx6-resource/rootfs/amod/usr/src/linux/include \
		   -I$(DIR_ROOT)inc/

SRCS = util.c devconf.c ispmgr.c

OBJS = $(SRCS:.c=.o)

all: $(OBJS)

etc : $(OBJS)

util.o : util.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@
	
devconf.o : devconf.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@	
	
ispmgr.o : ispmgr.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@	

clean:
	rm $(OBJS)
