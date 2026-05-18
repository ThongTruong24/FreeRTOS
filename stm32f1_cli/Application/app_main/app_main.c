#include "app_main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_board_slave.h"
#include "app_debug.h"
#include "app_gps.h"
#include "app_led.h"
#include "app_task.h"
#include "board_service.h"

void app_main_init(void)
{
    board_service_init();
    app_led_init();
    app_debug_init();
    app_gps_init();
    app_board_slave_init();
    app_task_init();
}

void app_main_start(void)
{
    app_task_start();

    vTaskStartScheduler();

    /* The scheduler should never return. */
    configASSERT(0);
}

void app_main(void)
{
    app_main_init();
    app_main_start();
}
