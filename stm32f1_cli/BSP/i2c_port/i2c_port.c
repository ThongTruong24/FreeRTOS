#include "i2c_port.h"
#include "main.h"
#include "param.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

static I2C_HandleTypeDef *i2c_port_get_handle(uint8_t instance)
{
    if (instance == 1U) {
        return &hi2c1;
    }

    if (instance == 2U) {
        return &hi2c2;
    }

    return 0;
}

uint8_t i2c_port_is_enabled(uint8_t instance)
{
    if (instance == 1U) {
        return (param_get(PARAM_I2C1_EN) != 0) ? 1U : 0U;
    }

    if (instance == 2U) {
        return (param_get(PARAM_I2C2_EN) != 0) ? 1U : 0U;
    }

    return 0;
}

uint8_t i2c_port_master_write(uint8_t instance, uint8_t addr7, const uint8_t *data, uint8_t len,
                             uint32_t timeout_ms)
{
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
}

uint8_t i2c_port_master_read(uint8_t instance, uint8_t addr7, uint8_t *data, uint8_t len,
                            uint32_t timeout_ms)
{
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
}

uint8_t i2c_port_slave_write(uint8_t instance, const uint8_t *data, uint8_t len,
                            uint32_t timeout_ms)
{
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U) ||
        (i2c_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Transmit(i2c,
                                   (uint8_t *)data,
                                   len,
                                   timeout_ms) == HAL_OK) ? 1U : 0U;
}

uint8_t i2c_port_start_slave_receive_it(uint8_t instance, uint8_t *data, uint8_t len)
{
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Receive_IT(i2c, data, len) == HAL_OK) ? 1U : 0U;
}

uint8_t i2c_port_start_slave_transmit_it(uint8_t instance, uint8_t *data, uint8_t len)
{
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    if ((i2c == 0) || (data == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_I2C_Slave_Transmit_IT(i2c, data, len) == HAL_OK) ? 1U : 0U;
}

uint8_t i2c_port_init(uint8_t instance)
{
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    return ((i2c != 0) && (HAL_I2C_Init(i2c) == HAL_OK)) ? 1U : 0U;
}

uint8_t i2c_port_deinit(uint8_t instance)
{
    I2C_HandleTypeDef *i2c = i2c_port_get_handle(instance);

    return ((i2c != 0) && (HAL_I2C_DeInit(i2c) == HAL_OK)) ? 1U : 0U;
}
