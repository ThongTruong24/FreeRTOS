################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Device_Driver/stm32_gpio_driver/stm32_gpio_driver.c 

OBJS += \
./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.o 

C_DEPS += \
./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Device_Driver/stm32_gpio_driver/%.o Device_Driver/stm32_gpio_driver/%.su Device_Driver/stm32_gpio_driver/%.cyclo: ../Device_Driver/stm32_gpio_driver/%.c Device_Driver/stm32_gpio_driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Application -I../Application/message -I../Application/param -I../Application/protocol -I../Application/app_board_slave -I../Application/app_debug -I../Application/app_led -I../Application/app_main -I../Application/app_task -I../BSP/board_config -I../BSP/debug_port -I../BSP/gpio_port -I../BSP/i2c_port -I../BSP/spi_port -I../BSP/uart_port -I../Device_Driver/stm32_gpio_driver -I../Device_Driver/stm32_i2c_driver -I../Device_Driver/stm32_spi_driver -I../Device_Driver/stm32_uart_driver -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/include" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/portable/GCC/ARM_CM3" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Device_Driver-2f-stm32_gpio_driver

clean-Device_Driver-2f-stm32_gpio_driver:
	-$(RM) ./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.cyclo ./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.d ./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.o ./Device_Driver/stm32_gpio_driver/stm32_gpio_driver.su

.PHONY: clean-Device_Driver-2f-stm32_gpio_driver

