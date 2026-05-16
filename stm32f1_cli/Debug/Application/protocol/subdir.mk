################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/protocol/protocol.c 

OBJS += \
./Application/protocol/protocol.o 

C_DEPS += \
./Application/protocol/protocol.d 


# Each subdirectory must supply rules for building sources it contributes
Application/protocol/%.o Application/protocol/%.su Application/protocol/%.cyclo: ../Application/protocol/%.c Application/protocol/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Application -I../Application/message -I../Application/param -I../Application/protocol -I../Application/app_board_slave -I../Application/app_debug -I../Application/app_led -I../Application/app_main -I../Application/app_task -I../BSP/board_config -I../BSP/debug_port -I../BSP/gpio_port -I../BSP/hal_callbacks -I../BSP/i2c_port -I../BSP/spi_port -I../BSP/uart_port -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/include" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/portable/GCC/ARM_CM3" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-protocol

clean-Application-2f-protocol:
	-$(RM) ./Application/protocol/protocol.cyclo ./Application/protocol/protocol.d ./Application/protocol/protocol.o ./Application/protocol/protocol.su

.PHONY: clean-Application-2f-protocol

