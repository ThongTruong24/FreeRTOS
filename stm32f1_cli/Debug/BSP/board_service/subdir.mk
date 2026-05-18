################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/board_service/board_service.c \
../BSP/board_service/param.c 

OBJS += \
./BSP/board_service/board_service.o \
./BSP/board_service/param.o 

C_DEPS += \
./BSP/board_service/board_service.d \
./BSP/board_service/param.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/board_service/%.o BSP/board_service/%.su BSP/board_service/%.cyclo: ../BSP/board_service/%.c BSP/board_service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Application -I../Application/message -I../BSP/board_service -I../BSP/board_devices -I../BSP/can_port -I../Application/protocol -I../Application/app_board_slave -I../Application/app_can_debug -I../Application/app_debug -I../Application/app_gps -I../Application/app_led -I../Application/app_main -I../Application/app_task -I../BSP/debug_port -I../BSP/gpio_port -I../BSP/hal_callbacks -I../BSP/rtos_port -I../BSP/slave_link_port -I../BSP/i2c_port -I../BSP/spi_port -I../BSP/uart_port -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/include" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/portable/GCC/ARM_CM3" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-BSP-2f-board_service

clean-BSP-2f-board_service:
	-$(RM) ./BSP/board_service/board_service.cyclo ./BSP/board_service/board_service.d ./BSP/board_service/board_service.o ./BSP/board_service/board_service.su ./BSP/board_service/param.cyclo ./BSP/board_service/param.d ./BSP/board_service/param.o ./BSP/board_service/param.su

.PHONY: clean-BSP-2f-board_service

