cmake_minimum_required(VERSION 3.5)
project(amod_mainapp)

set(CMAKE_C_COMPILER /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPLER /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-gcc++)
set(CMAKE_LINKER /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-ld)
set(CMAKE_AR /home/opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-ar)
set(CMAKE_RANLIB /opt/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-ranlib)

set(DIR_ROOT /home/shpark/workspace/project/amod-mainapp)
set(DIR_OUT ${DIR_ROOT}/out)
set(DIR_ROOTFS /opt/imx6-resource/rootfs/amod)

include_directories(${DIR_ROOT}/inc ${DIR_ROOTFS}/usr/include ${DIR_ROOTFS}/usr/include/directfb)
add_definitions(-c -g -std=gnu99 -O0 -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp)

set(SOURCE_FILES
    inc/coordtr/coordtr.h
    inc/core/magcomp.h
    inc/core/kvmf.h
    inc/core/device.h
    inc/core/evque.h
    inc/core/geodesic.h
    inc/core/guimgr.h
    inc/core/logger.h
    inc/core/sysctrl.h
    inc/core/taqdata_manager.h
    inc/core/taqmgr.h
    inc/core/msgproc.h
    inc/ds/list.h
    inc/ds/queue.h
    inc/ds/stack.h
    inc/etc/util.h
    inc/geomag/geomag.h
    inc/gui/gui_button.h
    inc/gui/gui_caption.h
    inc/gui/gui_dialog.h
    inc/gui/gui_dialog_creator.h
    inc/gui/gui_draw.h
    inc/gui/gui_grid.h
    inc/gui/gui_keyin_handler.h
    inc/gui/gui_menu.h
    inc/gui/gui_proc.h
    inc/gui/gui_qsel.h
    inc/gui/gui_slider.h
    inc/gui/gui_spinbox.h
    inc/gui/gui_str.h
    inc/gui/gui_submenu.h
    inc/gui/gui_svplot.h
    inc/gui/gui_window.h
    inc/gui/guiid.h
    inc/modules/camif.h
    inc/modules/display.h
    inc/modules/dmc.h
    inc/modules/eis.h
    inc/modules/gnss.h
    inc/modules/gpio.h
    inc/modules/ipu.h
    inc/modules/ircam.h
    inc/modules/irshtr.h
    inc/modules/lrf.h
    inc/modules/mcu.h
    inc/modules/uart.h
    inc/modules/vgss.h
    inc/morpho/morpho_api.h
    inc/morpho/morpho_error.h
    inc/morpho/morpho_get_image_size.h
    inc/morpho/morpho_image_data.h
    inc/morpho/morpho_image_data_ex.h
    inc/morpho/morpho_motion_data.h
    inc/morpho/morpho_motion_data_perspective.h
    inc/morpho/morpho_quadr_int.h
    inc/morpho/morpho_rect_int.h
    inc/morpho/morpho_video_refiner.h
    inc/morpho/morpho_video_refiner_internal.h
    inc/msg/kvmf_header.h
    inc/msg/dlp_header.h
    inc/msg/K00_1.h
    inc/msg/K02_9.h
    inc/msg/K02_22.h
    inc/msg/K02_37.h
    inc/amod.h
    inc/types.h
    inc/verinfo.h
    src/core/device.c
    src/core/evque.c
    src/core/geodesic.c
    src/core/guimgr.c
    src/core/logger.c
    src/core/magcomp.c
    src/core/main.c
    src/core/msgproc.c
    src/core/sysctrl.c
    src/core/taqdata_manager.c
    src/core/taqmgr.c
    src/ds/list.c
    src/ds/queue.c
    src/ds/stack.c
    src/etc/util.c
    src/gui/gui_button.c
    src/gui/gui_caption.c
    src/gui/gui_dialog.c
    src/gui/gui_dialog_creator.c
    src/gui/gui_draw.c
    src/gui/gui_grid.c
    src/gui/gui_keyin_handler.c
    src/gui/gui_menu.c
    src/gui/gui_qsel.c
    src/gui/gui_slider.c
    src/gui/gui_spinbox.c
    src/gui/gui_submenu.c
    src/gui/gui_svplot.c
    src/gui/gui_window.c
    src/modules/camif.c
    src/modules/display.c
    src/modules/dmc.c
    src/modules/eis.c
    src/modules/gnss.c
    src/modules/gpio.c
    src/modules/ipu.c
    src/modules/ircam.c
    src/modules/irshtr.c
    src/modules/lrf.c
    src/modules/mcu.c
    src/modules/uart.c
    src/modules/vgss.c)


add_executable(amod-mainapp ${SOURCE_FILES})