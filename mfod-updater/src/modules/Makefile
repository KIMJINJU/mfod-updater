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

SRCS = camif.c display.c dmc.c eis.c gnss.c gpio.c ipu.c ircam.c irshtr.c lrf.c mcu.c uart.c vgss.c sdcard.c

OBJS = $(SRCS:.c=.o)

all: $(OBJS)

modules : $(OBJS)

camif.o : camif.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

display.o : display.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

dmc.o : dmc.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

eis.o : eis.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gnss.o : gnss.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gpio.o : gpio.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

ipu.o : ipu.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

ircam.o: ircam.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

irshtr.o : irshtr.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

lrf.o : lrf.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

mcu.o : mcu.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

uart.o : uart.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

vgss.o : vgss.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@
	
sdcard.o : sdcard.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@


clean:
	rm $(OBJS)
