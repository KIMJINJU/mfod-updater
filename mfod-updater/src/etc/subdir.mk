################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/etc/util.c 

O_SRCS += \
../src/etc/util.o 

OBJS += \
./src/etc/util.o 

C_DEPS += \
./src/etc/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/etc/%.o: ../src/etc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


