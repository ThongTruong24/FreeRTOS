################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middleware/FreeRTOS-Kernel/croutine.c \
../Middleware/FreeRTOS-Kernel/event_groups.c \
../Middleware/FreeRTOS-Kernel/list.c \
../Middleware/FreeRTOS-Kernel/queue.c \
../Middleware/FreeRTOS-Kernel/stream_buffer.c \
../Middleware/FreeRTOS-Kernel/tasks.c \
../Middleware/FreeRTOS-Kernel/timers.c 

OBJS += \
./Middleware/FreeRTOS-Kernel/croutine.o \
./Middleware/FreeRTOS-Kernel/event_groups.o \
./Middleware/FreeRTOS-Kernel/list.o \
./Middleware/FreeRTOS-Kernel/queue.o \
./Middleware/FreeRTOS-Kernel/stream_buffer.o \
./Middleware/FreeRTOS-Kernel/tasks.o \
./Middleware/FreeRTOS-Kernel/timers.o 

C_DEPS += \
./Middleware/FreeRTOS-Kernel/croutine.d \
./Middleware/FreeRTOS-Kernel/event_groups.d \
./Middleware/FreeRTOS-Kernel/list.d \
./Middleware/FreeRTOS-Kernel/queue.d \
./Middleware/FreeRTOS-Kernel/stream_buffer.d \
./Middleware/FreeRTOS-Kernel/tasks.d \
./Middleware/FreeRTOS-Kernel/timers.d 


# Each subdirectory must supply rules for building sources it contributes
Middleware/FreeRTOS-Kernel/%.o Middleware/FreeRTOS-Kernel/%.su Middleware/FreeRTOS-Kernel/%.cyclo: ../Middleware/FreeRTOS-Kernel/%.c Middleware/FreeRTOS-Kernel/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Application -I../Application/message -I../BSP/board_service -I../BSP/board_devices -I../BSP/can_port -I../Application/protocol -I../Application/app_board_slave -I../Application/app_can_debug -I../Application/app_debug -I../Application/app_gps -I../Application/app_led -I../Application/app_main -I../Application/app_task -I../BSP/debug_port -I../BSP/gpio_port -I../BSP/hal_callbacks -I../BSP/rtos_port -I../BSP/slave_link_port -I../BSP/i2c_port -I../BSP/spi_port -I../BSP/uart_port -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/include" -I"D:/thuviec/FreeRTOS/stm32f1_cli/Middleware/FreeRTOS-Kernel/portable/GCC/ARM_CM3" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middleware-2f-FreeRTOS-2d-Kernel

clean-Middleware-2f-FreeRTOS-2d-Kernel:
	-$(RM) ./Middleware/FreeRTOS-Kernel/croutine.cyclo ./Middleware/FreeRTOS-Kernel/croutine.d ./Middleware/FreeRTOS-Kernel/croutine.o ./Middleware/FreeRTOS-Kernel/croutine.su ./Middleware/FreeRTOS-Kernel/event_groups.cyclo ./Middleware/FreeRTOS-Kernel/event_groups.d ./Middleware/FreeRTOS-Kernel/event_groups.o ./Middleware/FreeRTOS-Kernel/event_groups.su ./Middleware/FreeRTOS-Kernel/list.cyclo ./Middleware/FreeRTOS-Kernel/list.d ./Middleware/FreeRTOS-Kernel/list.o ./Middleware/FreeRTOS-Kernel/list.su ./Middleware/FreeRTOS-Kernel/queue.cyclo ./Middleware/FreeRTOS-Kernel/queue.d ./Middleware/FreeRTOS-Kernel/queue.o ./Middleware/FreeRTOS-Kernel/queue.su ./Middleware/FreeRTOS-Kernel/stream_buffer.cyclo ./Middleware/FreeRTOS-Kernel/stream_buffer.d ./Middleware/FreeRTOS-Kernel/stream_buffer.o ./Middleware/FreeRTOS-Kernel/stream_buffer.su ./Middleware/FreeRTOS-Kernel/tasks.cyclo ./Middleware/FreeRTOS-Kernel/tasks.d ./Middleware/FreeRTOS-Kernel/tasks.o ./Middleware/FreeRTOS-Kernel/tasks.su ./Middleware/FreeRTOS-Kernel/timers.cyclo ./Middleware/FreeRTOS-Kernel/timers.d ./Middleware/FreeRTOS-Kernel/timers.o ./Middleware/FreeRTOS-Kernel/timers.su

.PHONY: clean-Middleware-2f-FreeRTOS-2d-Kernel

