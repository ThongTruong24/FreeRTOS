#include "rtos_port.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_service.h"
#include "main.h"

void xPortSysTickHandler(void);

void rtos_port_on_systick(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    __disable_irq();

    while (1)
    {
        board_service_status_led_toggle_default();
        for (volatile uint32_t i = 0; i < 200000; i++);
    }
}
