#include "i2c_port.h"

#include "board_devices.h"
#include "param.h"

#if BOARD_HAS_ANY_I2C
static I2C_HandleTypeDef *i2c_port_get_handle(uint8_t instance)
{
    return board_devices_get_i2c(instance);
}
#endif

uint8_t i2c_port_is_enabled(uint8_t instance)
{
#if BOARD_HAS_ANY_I2C
    if (instance == 1U) {
        return (uint8_t)((i2c_port_get_handle(instance) != 0) &&
                         (param_get(PARAM_I2C1_PROTOCOL) != BUS_PROTOCOL_DISABLED));
    }

    if (instance == 2U) {
        return (uint8_t)((i2c_port_get_handle(instance) != 0) &&
                         (param_get(PARAM_I2C2_PROTOCOL) != BUS_PROTOCOL_DISABLED));
    }

    return 0;
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t i2c_port_master_write(uint8_t instance, uint8_t addr7, const uint8_t *data, uint8_t len,
                             uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U) ||
        (i2c_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    return (HAL_I2C_Master_Transmit(i2c,
                                    (uint16_t)(addr7 << 1),
                                    (uint8_t *)data,
                                    len,
                                    timeout_ms) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)addr7;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t i2c_port_master_read(uint8_t instance, uint8_t addr7, uint8_t *data, uint8_t len,
                            uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U) ||
        (i2c_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    return (HAL_I2C_Master_Receive(i2c,
                                   (uint16_t)(addr7 << 1),
                                   data,
                                   len,
                                   timeout_ms) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)addr7;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t i2c_port_slave_write(uint8_t instance, const uint8_t *data, uint8_t len,
                            uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U) ||
        (i2c_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Transmit(i2c,
                                   (uint8_t *)data,
                                   len,
                                   timeout_ms) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t i2c_port_start_slave_receive_it(uint8_t instance, uint8_t *data, uint8_t len)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Receive_IT(i2c, data, len) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    return 0U;
#endif
}

uint8_t i2c_port_start_slave_transmit_it(uint8_t instance, uint8_t *data, uint8_t len)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Transmit_IT(i2c, data, len) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    return 0U;
#endif
}

uint8_t i2c_port_start_slave_seq_receive_it(uint8_t instance, uint8_t *data, uint8_t len)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Seq_Receive_IT(i2c,
                                         data,
                                         len,
                                         I2C_FIRST_AND_LAST_FRAME) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    return 0U;
#endif
}

uint8_t i2c_port_start_slave_seq_transmit_it(uint8_t instance, uint8_t *data, uint8_t len)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Seq_Transmit_IT(i2c,
                                          data,
                                          len,
                                          I2C_FIRST_AND_LAST_FRAME) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    return 0U;
#endif
}

uint8_t i2c_port_enable_listen_it(uint8_t instance)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    return ((i2c != 0) && (HAL_I2C_EnableListen_IT(i2c) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}

#if BOARD_HAS_ANY_I2C
uint8_t i2c_port_instance_from_handle(const I2C_HandleTypeDef *i2c)
{
    return board_devices_i2c_instance_from_handle(i2c);
}
#endif

uint8_t i2c_port_init(uint8_t instance)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    return ((i2c != 0) && (HAL_I2C_Init(i2c) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t i2c_port_deinit(uint8_t instance)
{
#if BOARD_HAS_ANY_I2C
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    return ((i2c != 0) && (HAL_I2C_DeInit(i2c) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}
