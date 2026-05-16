################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middleware/FreeRTOS-Kernel/portable/MemMang/subdir.mk
-include Middleware/FreeRTOS-Kernel/portable/GCC/ARM_CM3/subdir.mk
-include Middleware/FreeRTOS-Kernel/subdir.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include BSP/uart_port/subdir.mk
-include BSP/spi_port/subdir.mk
-include BSP/i2c_port/subdir.mk
-include BSP/hal_callbacks/subdir.mk
-include BSP/gpio_port/subdir.mk
-include BSP/debug_port/subdir.mk
-include BSP/board_config/subdir.mk
-include Application/protocol/subdir.mk
-include Application/param/subdir.mk
-include Application/app_task/subdir.mk
-include Application/app_main/subdir.mk
-include Application/app_led/subdir.mk
-include Application/app_debug/subdir.mk
-include Application/app_board_slave/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := stm32f1_cli
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
stm32f1_cli.elf \

MAP_FILES += \
stm32f1_cli.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
stm32f1_cli.list \


# All Target
all: main-build

# Main-build Target
main-build: stm32f1_cli.elf secondary-outputs

# Tool invocations
stm32f1_cli.elf stm32f1_cli.map: $(OBJS) $(USER_OBJS) D:\thuviec\FreeRTOS\stm32f1_cli\STM32F103C8TX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "stm32f1_cli.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\thuviec\FreeRTOS\stm32f1_cli\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="stm32f1_cli.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

stm32f1_cli.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "stm32f1_cli.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout stm32f1_cli.elf stm32f1_cli.list stm32f1_cli.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
