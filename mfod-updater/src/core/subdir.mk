################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/device.c \
../src/core/evque.c \
../src/core/geodesic.c \
../src/core/guimgr.c \
../src/core/logger.c \
../src/core/magcomp.c \
../src/core/main.c \
../src/core/msgproc.c \
../src/core/sysctrl.c \
../src/core/taqdata_manager.c \
../src/core/taqmgr.c 

O_SRCS += \
../src/core/device.o \
../src/core/evque.o \
../src/core/geodesic.o \
../src/core/guimgr.o \
../src/core/logger.o \
../src/core/magcomp.o \
../src/core/main.o \
../src/core/msgproc.o \
../src/core/sysctrl.o \
../src/core/taqdata_manager.o \
../src/core/taqmgr.o 

OBJS += \
./src/core/device.o \
./src/core/evque.o \
./src/core/geodesic.o \
./src/core/guimgr.o \
./src/core/logger.o \
./src/core/magcomp.o \
./src/core/main.o \
./src/core/msgproc.o \
./src/core/sysctrl.o \
./src/core/taqdata_manager.o \
./src/core/taqmgr.o 

C_DEPS += \
./src/core/device.d \
./src/core/evque.d \
./src/core/geodesic.d \
./src/core/guimgr.d \
./src/core/logger.d \
./src/core/magcomp.d \
./src/core/main.d \
./src/core/msgproc.d \
./src/core/sysctrl.d \
./src/core/taqdata_manager.d \
./src/core/taqmgr.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/%.o: ../src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


