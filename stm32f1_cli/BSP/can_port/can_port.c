#include "can_port.h"

#include "param.h"

static can_protocol_t can_port_protocol(uint8_t instance);

uint8_t can_port_is_enabled(uint8_t instance)
{
#if BOARD_HAS_ANY_CAN
    return (uint8_t)((board_devices_get_can(instance) != 0) &&
                     (can_port_protocol(instance) != CAN_PROTOCOL_DISABLED));
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t can_port_start(uint8_t instance)
{
#if BOARD_HAS_ANY_CAN
    CAN_HandleTypeDef *can = board_devices_get_can(instance);
    uint32_t notifications;

    if ((can == 0) || (can_port_is_enabled(instance) == 0U))
    {
        return 0U;
    }

    notifications = CAN_IT_TX_MAILBOX_EMPTY |
                    CAN_IT_ERROR_WARNING |
                    CAN_IT_ERROR_PASSIVE |
                    CAN_IT_BUSOFF |
                    CAN_IT_LAST_ERROR_CODE |
                    CAN_IT_ERROR;

    return (uint8_t)((HAL_CAN_Start(can) == HAL_OK) &&
                     (HAL_CAN_ActivateNotification(can, notifications) == HAL_OK));
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t can_port_stop(uint8_t instance)
{
#if BOARD_HAS_ANY_CAN
    CAN_HandleTypeDef *can = board_devices_get_can(instance);

    return (uint8_t)((can != 0) && (HAL_CAN_Stop(can) == HAL_OK));
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t can_port_write_std(uint8_t instance,
                           uint16_t std_id,
                           const uint8_t *data,
                           uint8_t len)
{
#if BOARD_HAS_ANY_CAN
    CAN_HandleTypeDef *can = board_devices_get_can(instance);
    CAN_TxHeaderTypeDef header = {0};
    uint32_t mailbox;

    if ((can == 0) ||
        (can_port_is_enabled(instance) == 0U) ||
        (std_id > 0x7FFU) ||
        (len > 8U) ||
        ((data == 0) && (len > 0U)))
    {
        return 0U;
    }

    header.StdId = std_id;
    header.ExtId = 0U;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = len;
    header.TransmitGlobalTime = DISABLE;

    return (HAL_CAN_AddTxMessage(can,
                                 &header,
                                 (uint8_t *)data,
                                 &mailbox) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)std_id;
    (void)data;
    (void)len;
    return 0U;
#endif
}

uint8_t can_port_tx_mailboxes_are_full(uint8_t instance)
{
#if BOARD_HAS_ANY_CAN
    CAN_HandleTypeDef *can = board_devices_get_can(instance);

    return (uint8_t)((can != 0) &&
                     (HAL_CAN_GetTxMailboxesFreeLevel(can) == 0U));
#else
    (void)instance;
    return 0U;
#endif
}

uint32_t can_port_take_error(uint8_t instance)
{
#if BOARD_HAS_ANY_CAN
    CAN_HandleTypeDef *can = board_devices_get_can(instance);
    uint32_t error;

    if (can == 0)
    {
        return HAL_CAN_ERROR_NOT_INITIALIZED;
    }

    error = HAL_CAN_GetError(can);

    if (error != HAL_CAN_ERROR_NONE)
    {
        (void)HAL_CAN_ResetError(can);
    }

    return error;
#else
    (void)instance;
    return 0U;
#endif
}

#if BOARD_HAS_ANY_CAN
uint8_t can_port_instance_from_handle(const CAN_HandleTypeDef *can)
{
    return board_devices_can_instance_from_handle(can);
}
#endif

static can_protocol_t can_port_protocol(uint8_t instance)
{
    if (instance == 1U)
    {
        return (can_protocol_t)param_get(PARAM_CAN1_PROTOCOL);
    }

    if (instance == 2U)
    {
        return (can_protocol_t)param_get(PARAM_CAN2_PROTOCOL);
    }

    return CAN_PROTOCOL_DISABLED;
}
