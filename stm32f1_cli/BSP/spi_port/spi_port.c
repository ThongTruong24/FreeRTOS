#include "spi_port.h"
#include "main.h"
#include "param.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

static SPI_HandleTypeDef *spi_port_get_handle(uint8_t instance)
{
    if (instance == 1U) {
        return &hspi1;
    }

    if (instance == 2U) {
        return &hspi2;
    }

    return 0;
}

uint8_t spi_port_is_enabled(uint8_t instance)
{
    if (instance == 1U) {
        return (param_get(PARAM_SPI1_EN) != 0) ? 1U : 0U;
    }

    if (instance == 2U) {
        return (param_get(PARAM_SPI2_EN) != 0) ? 1U : 0U;
    }

    return 0;
}

uint8_t spi_port_write(uint8_t instance, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (data == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_Transmit(spi, (uint8_t *)data, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
}

uint8_t spi_port_read(uint8_t instance, uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (data == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_Receive(spi, data, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
}

uint8_t spi_port_transfer(uint8_t instance, const uint8_t *tx, uint8_t *rx, uint8_t len,
                         uint32_t timeout_ms)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (tx == 0) || (rx == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_TransmitReceive(spi, (uint8_t *)tx, rx, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
}

uint8_t spi_port_is_slave(uint8_t instance)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (spi->Init.Mode == SPI_MODE_SLAVE)) ? 1U : 0U;
}

uint8_t spi_port_start_slave_transfer_it(uint8_t instance, uint8_t *tx, uint8_t *rx,
                                         uint8_t len)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    if ((spi == 0) || (tx == 0) || (rx == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_SPI_TransmitReceive_IT(spi, tx, rx, len) == HAL_OK) ? 1U : 0U;
}

uint8_t spi_port_abort_it(uint8_t instance)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_Abort_IT(spi) == HAL_OK)) ? 1U : 0U;
}

uint8_t spi_port_init(uint8_t instance)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_Init(spi) == HAL_OK)) ? 1U : 0U;
}

uint8_t spi_port_deinit(uint8_t instance)
{
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_DeInit(spi) == HAL_OK)) ? 1U : 0U;
}
