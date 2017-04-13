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

SRCS = gui_button.c gui_caption.c gui_dialog.c gui_dialog_creator.c gui_draw.c gui_grid.c gui_keyin_handler.c gui_menu.c gui_qsel.c gui_slider.c gui_spinbox.c gui_submenu.c gui_svplot.c gui_window.c

OBJS = $(SRCS:.c=.o)

gui : $(OBJS)

gui_button.o : gui_button.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_caption.o : gui_caption.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_dialog.o : gui_dialog.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_dialog_creator.o : gui_dialog_creator.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_draw.o : gui_draw.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_grid.o : gui_grid.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_keyin_handler.o : gui_keyin_handler.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_menu.o : gui_menu.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_qsel.o : gui_qsel.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_slider.o : gui_slider.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_spinbox.o : gui_spinbox.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_submenu.o : gui_submenu.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_svplot.o : gui_svplot.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

gui_window.o : gui_window.c
	$(CC) $(CFLAGS) $(DIR_INC) $< -o $@

clean:
	rm $(OBJS)
