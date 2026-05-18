#include "slave_link_port.h"

#include <string.h>

#include "app_board_slave.h"
#include "i2c_port.h"
#include "main.h"
#include "protocol.h"
#include "spi_port.h"
#include "uart_port.h"

#define SLAVE_LINK_PORT_TRANSACTION_SIZE    PROTOCOL_FRAME_MAX_LEN

static uint8_t s_tx_frame[SLAVE_LINK_PORT_TRANSACTION_SIZE];
static uint8_t s_rx_frame[SLAVE_LINK_PORT_TRANSACTION_SIZE];
static uint8_t s_master_tx_frame[SLAVE_LINK_PORT_TRANSACTION_SIZE];
static uint8_t s_master_rx_frame[SLAVE_LINK_PORT_TRANSACTION_SIZE];

static void slave_link_port_clear_frame(uint8_t *frame);
static void slave_link_port_prepare_frame(uint8_t *dst, const uint8_t *src, uint8_t len);
static void slave_link_port_feed_frame(const uint8_t *frame);
static void slave_link_port_feed_frame_from_isr(const uint8_t *frame);

uint8_t slave_link_port_stage_tx_frame(const uint8_t *data, uint8_t len)
{
    if ((data == 0) || (len == 0U) || (len > SLAVE_LINK_PORT_TRANSACTION_SIZE))
    {
        return 0U;
    }

    slave_link_port_prepare_frame(s_tx_frame, data, len);
    return 1U;
}

uint8_t slave_link_port_uart_write(uint8_t instance,
                                   const uint8_t *data,
                                   uint8_t len,
                                   uint32_t timeout_ms)
{
    return uart_port_write(instance, data, len, timeout_ms);
}

void slave_link_port_uart_rx_byte_from_isr(uint8_t byte)
{
    app_board_slave_rx_byte_from_isr(byte);
}

uint8_t slave_link_port_spi_master_write(uint8_t instance,
                                         const uint8_t *data,
                                         uint8_t len,
                                         uint32_t timeout_ms)
{
    if ((data == 0) || (len == 0U) || (len > SLAVE_LINK_PORT_TRANSACTION_SIZE))
    {
        return 0U;
    }

    slave_link_port_prepare_frame(s_master_tx_frame, data, len);
    slave_link_port_clear_frame(s_master_rx_frame);

    if (spi_port_transfer(instance,
                          s_master_tx_frame,
                          s_master_rx_frame,
                          sizeof(s_master_tx_frame),
                          timeout_ms) == 0U)
    {
        return 0U;
    }

    slave_link_port_feed_frame(s_master_rx_frame);
    return 1U;
}

uint8_t slave_link_port_spi_master_poll_rx(uint8_t instance, uint32_t timeout_ms)
{
    slave_link_port_clear_frame(s_master_tx_frame);
    slave_link_port_clear_frame(s_master_rx_frame);

    if (spi_port_transfer(instance,
                          s_master_tx_frame,
                          s_master_rx_frame,
                          sizeof(s_master_tx_frame),
                          timeout_ms) == 0U)
    {
        return 0U;
    }

    slave_link_port_feed_frame(s_master_rx_frame);
    return 1U;
}

uint8_t slave_link_port_spi_slave_start(uint8_t instance)
{
    slave_link_port_clear_frame(s_rx_frame);
    return spi_port_start_slave_transfer_it(instance,
                                            s_tx_frame,
                                            s_rx_frame,
                                            sizeof(s_rx_frame));
}

void slave_link_port_spi_slave_transfer_complete_from_isr(uint8_t instance)
{
    slave_link_port_feed_frame_from_isr(s_rx_frame);
    slave_link_port_clear_frame(s_tx_frame);
    slave_link_port_clear_frame(s_rx_frame);
    (void)slave_link_port_spi_slave_start(instance);
}

void slave_link_port_spi_slave_error_from_isr(uint8_t instance)
{
    (void)spi_port_abort_it(instance);
    slave_link_port_clear_frame(s_rx_frame);
    (void)slave_link_port_spi_slave_start(instance);
}

uint8_t slave_link_port_i2c_master_write(uint8_t instance,
                                         uint8_t addr7,
                                         const uint8_t *data,
                                         uint8_t len,
                                         uint32_t timeout_ms)
{
    if ((data == 0) || (len == 0U) || (len > SLAVE_LINK_PORT_TRANSACTION_SIZE))
    {
        return 0U;
    }

    slave_link_port_prepare_frame(s_master_tx_frame, data, len);
    return i2c_port_master_write(instance,
                                 addr7,
                                 s_master_tx_frame,
                                 sizeof(s_master_tx_frame),
                                 timeout_ms);
}

uint8_t slave_link_port_i2c_master_poll_rx(uint8_t instance,
                                           uint8_t addr7,
                                           uint32_t timeout_ms)
{
    slave_link_port_clear_frame(s_master_rx_frame);

    if (i2c_port_master_read(instance,
                             addr7,
                             s_master_rx_frame,
                             sizeof(s_master_rx_frame),
                             timeout_ms) == 0U)
    {
        return 0U;
    }

    slave_link_port_feed_frame(s_master_rx_frame);
    return 1U;
}

uint8_t slave_link_port_i2c_slave_start(uint8_t instance)
{
    slave_link_port_clear_frame(s_rx_frame);
    return i2c_port_enable_listen_it(instance);
}

void slave_link_port_i2c_slave_addr_from_isr(uint8_t instance, uint8_t transfer_direction)
{
#if BOARD_HAS_ANY_I2C
    if (transfer_direction == I2C_DIRECTION_TRANSMIT)
    {
        slave_link_port_clear_frame(s_rx_frame);
        (void)i2c_port_start_slave_seq_receive_it(instance,
                                                  s_rx_frame,
                                                  sizeof(s_rx_frame));
    }
    else
    {
        (void)i2c_port_start_slave_seq_transmit_it(instance,
                                                   s_tx_frame,
                                                   sizeof(s_tx_frame));
    }
#else
    (void)instance;
    (void)transfer_direction;
#endif
}

void slave_link_port_i2c_slave_rx_complete_from_isr(uint8_t instance)
{
    slave_link_port_feed_frame_from_isr(s_rx_frame);
    slave_link_port_clear_frame(s_rx_frame);
    (void)i2c_port_enable_listen_it(instance);
}

void slave_link_port_i2c_slave_tx_complete_from_isr(uint8_t instance)
{
    slave_link_port_clear_frame(s_tx_frame);
    (void)i2c_port_enable_listen_it(instance);
}

void slave_link_port_i2c_slave_error_from_isr(uint8_t instance)
{
    slave_link_port_clear_frame(s_rx_frame);
    (void)i2c_port_enable_listen_it(instance);
}

static void slave_link_port_clear_frame(uint8_t *frame)
{
    memset(frame, 0, SLAVE_LINK_PORT_TRANSACTION_SIZE);
}

static void slave_link_port_prepare_frame(uint8_t *dst, const uint8_t *src, uint8_t len)
{
    slave_link_port_clear_frame(dst);
    memcpy(dst, src, len);
}

static void slave_link_port_feed_frame(const uint8_t *frame)
{
    uint8_t i;

    for (i = 0U; i < SLAVE_LINK_PORT_TRANSACTION_SIZE; i++)
    {
        app_board_slave_rx_byte(frame[i]);
    }
}

static void slave_link_port_feed_frame_from_isr(const uint8_t *frame)
{
    uint8_t i;

    for (i = 0U; i < SLAVE_LINK_PORT_TRANSACTION_SIZE; i++)
    {
        app_board_slave_rx_byte_from_isr(frame[i]);
    }
}
