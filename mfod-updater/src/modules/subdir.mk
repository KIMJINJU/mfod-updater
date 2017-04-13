################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/modules/camif.c \
../src/modules/display.c \
../src/modules/dmc.c \
../src/modules/eis.c \
../src/modules/gnss.c \
../src/modules/gpio.c \
../src/modules/ipu.c \
../src/modules/ircam.c \
../src/modules/irshtr.c \
../src/modules/lrf.c \
../src/modules/mcu.c \
../src/modules/uart.c \
../src/modules/vgss.c 

O_SRCS += \
../src/modules/camif.o \
../src/modules/display.o \
../src/modules/dmc.o \
../src/modules/eis.o \
../src/modules/gnss.o \
../src/modules/gpio.o \
../src/modules/ipu.o \
../src/modules/ircam.o \
../src/modules/irshtr.o \
../src/modules/lrf.o \
../src/modules/mcu.o \
../src/modules/uart.o \
../src/modules/vgss.o 

OBJS += \
./src/modules/camif.o \
./src/modules/display.o \
./src/modules/dmc.o \
./src/modules/eis.o \
./src/modules/gnss.o \
./src/modules/gpio.o \
./src/modules/ipu.o \
./src/modules/ircam.o \
./src/modules/irshtr.o \
./src/modules/lrf.o \
./src/modules/mcu.o \
./src/modules/uart.o \
./src/modules/vgss.o 

C_DEPS += \
./src/modules/camif.d \
./src/modules/display.d \
./src/modules/dmc.d \
./src/modules/eis.d \
./src/modules/gnss.d \
./src/modules/gpio.d \
./src/modules/ipu.d \
./src/modules/ircam.d \
./src/modules/irshtr.d \
./src/modules/lrf.d \
./src/modules/mcu.d \
./src/modules/uart.d \
./src/modules/vgss.d 


# Each subdirectory must supply rules for building sources it contributes
src/modules/%.o: ../src/modules/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


