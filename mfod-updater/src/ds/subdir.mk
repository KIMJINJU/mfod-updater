################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ds/list.c \
../src/ds/queue.c \
../src/ds/stack.c 

O_SRCS += \
../src/ds/list.o \
../src/ds/queue.o \
../src/ds/stack.o 

OBJS += \
./src/ds/list.o \
./src/ds/queue.o \
./src/ds/stack.o 

C_DEPS += \
./src/ds/list.d \
./src/ds/queue.d \
./src/ds/stack.d 


# Each subdirectory must supply rules for building sources it contributes
src/ds/%.o: ../src/ds/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


