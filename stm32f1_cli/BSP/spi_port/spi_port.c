#include "spi_port.h"

#include "board_devices.h"
#include "param.h"

#if BOARD_HAS_ANY_SPI
static SPI_HandleTypeDef *spi_port_get_handle(uint8_t instance)
{
    return board_devices_get_spi(instance);
}
#endif

uint8_t spi_port_is_enabled(uint8_t instance)
{
#if BOARD_HAS_ANY_SPI
    if (instance == 1U) {
        return (uint8_t)((spi_port_get_handle(instance) != 0) &&
                         (param_get(PARAM_SPI1_PROTOCOL) != BUS_PROTOCOL_DISABLED));
    }

    if (instance == 2U) {
        return (uint8_t)((spi_port_get_handle(instance) != 0) &&
                         (param_get(PARAM_SPI2_PROTOCOL) != BUS_PROTOCOL_DISABLED));
    }

    return 0;
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t spi_port_write(uint8_t instance, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (data == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_Transmit(spi, (uint8_t *)data, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t spi_port_read(uint8_t instance, uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (data == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_Receive(spi, data, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t spi_port_transfer(uint8_t instance, const uint8_t *tx, uint8_t *rx, uint8_t len,
                         uint32_t timeout_ms)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);
    HAL_StatusTypeDef status;

    if ((spi == 0) || (tx == 0) || (rx == 0) || (len == 0U) ||
        (spi_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    status = HAL_SPI_TransmitReceive(spi, (uint8_t *)tx, rx, len, timeout_ms);
    __HAL_SPI_DISABLE(spi);
    return (status == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)tx;
    (void)rx;
    (void)len;
    (void)timeout_ms;
    return 0U;
#endif
}

uint8_t spi_port_is_slave(uint8_t instance)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (spi->Init.Mode == SPI_MODE_SLAVE)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t spi_port_start_slave_transfer_it(uint8_t instance, uint8_t *tx, uint8_t *rx,
                                         uint8_t len)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    if ((spi == 0) || (tx == 0) || (rx == 0) || (len == 0U)) {
        return 0;
    }

    return (HAL_SPI_TransmitReceive_IT(spi, tx, rx, len) == HAL_OK) ? 1U : 0U;
#else
    (void)instance;
    (void)tx;
    (void)rx;
    (void)len;
    return 0U;
#endif
}

uint8_t spi_port_abort_it(uint8_t instance)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_Abort_IT(spi) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}

#if BOARD_HAS_ANY_SPI
uint8_t spi_port_instance_from_handle(const SPI_HandleTypeDef *spi)
{
    return board_devices_spi_instance_from_handle(spi);
}
#endif

uint8_t spi_port_init(uint8_t instance)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_Init(spi) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}

uint8_t spi_port_deinit(uint8_t instance)
{
#if BOARD_HAS_ANY_SPI
    SPI_HandleTypeDef *spi = spi_port_get_handle(instance);

    return ((spi != 0) && (HAL_SPI_DeInit(spi) == HAL_OK)) ? 1U : 0U;
#else
    (void)instance;
    return 0U;
#endif
}
