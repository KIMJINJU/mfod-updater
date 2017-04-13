################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gui/gui_button.c \
../src/gui/gui_caption.c \
../src/gui/gui_dialog.c \
../src/gui/gui_dialog_creator.c \
../src/gui/gui_draw.c \
../src/gui/gui_grid.c \
../src/gui/gui_keyin_handler.c \
../src/gui/gui_menu.c \
../src/gui/gui_qsel.c \
../src/gui/gui_slider.c \
../src/gui/gui_spinbox.c \
../src/gui/gui_submenu.c \
../src/gui/gui_svplot.c \
../src/gui/gui_window.c 

O_SRCS += \
../src/gui/gui_button.o \
../src/gui/gui_caption.o \
../src/gui/gui_dialog.o \
../src/gui/gui_dialog_creator.o \
../src/gui/gui_draw.o \
../src/gui/gui_grid.o \
../src/gui/gui_keyin_handler.o \
../src/gui/gui_menu.o \
../src/gui/gui_qsel.o \
../src/gui/gui_slider.o \
../src/gui/gui_spinbox.o \
../src/gui/gui_submenu.o \
../src/gui/gui_svplot.o \
../src/gui/gui_window.o 

OBJS += \
./src/gui/gui_button.o \
./src/gui/gui_caption.o \
./src/gui/gui_dialog.o \
./src/gui/gui_dialog_creator.o \
./src/gui/gui_draw.o \
./src/gui/gui_grid.o \
./src/gui/gui_keyin_handler.o \
./src/gui/gui_menu.o \
./src/gui/gui_qsel.o \
./src/gui/gui_slider.o \
./src/gui/gui_spinbox.o \
./src/gui/gui_submenu.o \
./src/gui/gui_svplot.o \
./src/gui/gui_window.o 

C_DEPS += \
./src/gui/gui_button.d \
./src/gui/gui_caption.d \
./src/gui/gui_dialog.d \
./src/gui/gui_dialog_creator.d \
./src/gui/gui_draw.d \
./src/gui/gui_grid.d \
./src/gui/gui_keyin_handler.d \
./src/gui/gui_menu.d \
./src/gui/gui_qsel.d \
./src/gui/gui_slider.d \
./src/gui/gui_spinbox.d \
./src/gui/gui_submenu.d \
./src/gui/gui_svplot.d \
./src/gui/gui_window.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/%.o: ../src/gui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


