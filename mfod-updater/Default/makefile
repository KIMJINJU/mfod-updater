################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include src/modules/subdir.mk
-include src/gui/subdir.mk
-include src/etc/subdir.mk
-include src/ds/subdir.mk
-include src/core/subdir.mk
-include obj/subdir.mk
-include cmake-build-debug/CMakeFiles/3.6.2/CompilerIdC/subdir.mk
-include cmake-build-debug/CMakeFiles/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: mfod-updater

# Tool invocations
mfod-updater: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -o "mfod-updater" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) mfod-updater
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
