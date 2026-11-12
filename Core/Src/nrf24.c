#include "nrf24.h"
#include <string.h>

/* nRF24L01+ commands */
#define CMD_R_REGISTER       0x00U
#define CMD_W_REGISTER       0x20U
#define CMD_R_RX_PAYLOAD     0x61U
#define CMD_W_TX_PAYLOAD     0xA0U
#define CMD_FLUSH_TX         0xE1U
#define CMD_FLUSH_RX         0xE2U
#define CMD_R_RX_PL_WID      0x60U
#define CMD_ACTIVATE         0x50U
#define CMD_NOP              0xFFU

/* Registers */
#define REG_CONFIG           0x00U
#define REG_EN_AA            0x01U
#define REG_EN_RXADDR        0x02U
#define REG_SETUP_AW         0x03U
#define REG_SETUP_RETR       0x04U
#define REG_RF_CH             0x05U
#define REG_RF_SETUP         0x06U
#define REG_STATUS           0x07U
#define REG_RX_ADDR_P0       0x0AU
#define REG_TX_ADDR          0x10U
#define REG_RX_PW_P0         0x11U
#define REG_FIFO_STATUS      0x17U
#define REG_DYNPD            0x1CU
#define REG_FEATURE          0x1DU

/* CONFIG bits */
#define CONFIG_PRIM_RX       0x01U
#define CONFIG_PWR_UP        0x02U
#define CONFIG_CRCO          0x04U
#define CONFIG_EN_CRC        0x08U

/* STATUS bits */
#define STATUS_RX_DR         0x40U
#define STATUS_TX_DS         0x20U
#define STATUS_MAX_RT        0x10U

#define FIFO_RX_EMPTY        0x01U
#define FEATURE_EN_DPL       0x04U
#define FEATURE_EN_ACK_PAY   0x02U

#define SPI_TIMEOUT_MS       100U


/* =========================================================
 * PRIVATE GPIO FUNCTIONS
 * ========================================================= */

static void csn_low(NRF24 *d)
{
    HAL_GPIO_WritePin(d->csn_port, d->csn_pin, GPIO_PIN_RESET);
}

static void csn_high(NRF24 *d)
{
    HAL_GPIO_WritePin(d->csn_port, d->csn_pin, GPIO_PIN_SET);
}

static void ce_low(NRF24 *d)
{
    HAL_GPIO_WritePin(d->ce_port, d->ce_pin, GPIO_PIN_RESET);
}

static void ce_high(NRF24 *d)
{
    HAL_GPIO_WritePin(d->ce_port, d->ce_pin, GPIO_PIN_SET);
}

/* =========================================================
 * PRIVATE SPI FUNCTIONS
 * ========================================================= */

static uint8_t spi_byte(NRF24 *d, uint8_t value)
{
    uint8_t rx = 0xFFU;

    if (HAL_SPI_TransmitReceive(d->hspi, &value, &rx, 1,
                                SPI_TIMEOUT_MS) != HAL_OK)
    {
        d->status = 0x00U;
    }

    return rx;
}

static uint8_t read_reg(NRF24 *d, uint8_t reg)
{
    uint8_t value;

    csn_low(d);
    spi_byte(d, CMD_R_REGISTER | (reg & 0x1FU));
    value = spi_byte(d, CMD_NOP);
    csn_high(d);

    return value;
}

static void write_reg(NRF24 *d, uint8_t reg, uint8_t value)
{
    csn_low(d);
    spi_byte(d, CMD_W_REGISTER | (reg & 0x1FU));
    spi_byte(d, value);
    csn_high(d);
}

static void write_buf(NRF24 *d, uint8_t reg,
                      const uint8_t *buf, uint8_t length)
{
    csn_low(d);
    spi_byte(d, CMD_W_REGISTER | (reg & 0x1FU));

    for (uint8_t i = 0; i < length; i++)
    {
        spi_byte(d, buf[i]);
    }

    csn_high(d);
}

static void read_buf(NRF24 *d, uint8_t cmd,
                     uint8_t *buf, uint8_t length)
{
    csn_low(d);
    spi_byte(d, cmd);

    for (uint8_t i = 0; i < length; i++)
    {
        buf[i] = spi_byte(d, CMD_NOP);
    }

    csn_high(d);
}

static void command(NRF24 *d, uint8_t cmd)
{
    csn_low(d);
    spi_byte(d, cmd);
    csn_high(d);
}

uint8_t NRF24_GetStatus(NRF24 *dev)
{
    if (dev == NULL || dev->hspi == NULL)
    {
        return 0x00U;
    }
  
    csn_low(dev);
    dev->status = spi_byte(dev, CMD_NOP);
    csn_high(dev);

    return dev->status;
}

void NRF24_FlushRx(NRF24 *dev)
{
    if (dev != NULL)
    {
        command(dev, CMD_FLUSH_RX);
    }
}

void NRF24_FlushTx(NRF24 *dev)
{
    if (dev != NULL)
    {
        command(dev, CMD_FLUSH_TX);
    }
}

static void clear_irqs(NRF24 *d)
{
    /* STATUS IRQ flags are cleared by writing ones. */
    write_reg(d, REG_STATUS,
              STATUS_RX_DR | STATUS_TX_DS | STATUS_MAX_RT);
}

static void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* =========================================================
 * INITIALIZATION
 * ========================================================= */

NRF24_Status NRF24_Init(NRF24 *dev,
                        SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *csn_port, uint16_t csn_pin,
                        GPIO_TypeDef *ce_port, uint16_t ce_pin,
                        const uint8_t address[5])
{
    if (dev == NULL || hspi == NULL ||
        csn_port == NULL || ce_port == NULL || address == NULL)
    {
        return NRF24_INVALID_PARAM;
    }

    dev->hspi = hspi;
    dev->csn_port = csn_port;
    dev->csn_pin = csn_pin;
    dev->ce_port = ce_port;
    dev->ce_pin = ce_pin;

    memcpy(dev->address, address, 5U);

    dev->channel = 76U;
    dev->data_rate = 0U; /* 1 Mbps */
    dev->tx_power = 3U;  /* 0 dBm */

    ce_low(dev);
    csn_high(dev);
    delay_ms(5U);

    /* Power down before configuration. */
    write_reg(dev, REG_CONFIG, CONFIG_EN_CRC | CONFIG_CRCO);
    delay_ms(5U);

    /* Five-byte addresses. */
    write_reg(dev, REG_SETUP_AW, 0x03U);

    /* Auto-ack on pipe 0; 500 us delay, 10 retransmissions. */
    write_reg(dev, REG_EN_AA, 0x01U);
    write_reg(dev, REG_EN_RXADDR, 0x01U);
    write_reg(dev, REG_SETUP_RETR, 0x5AU);

    write_reg(dev, REG_RF_CH, dev->channel);
    write_reg(dev, REG_RF_SETUP, 0x06U); /* 1 Mbps, 0 dBm */

    write_buf(dev, REG_TX_ADDR, dev->address, 5U);
    write_buf(dev, REG_RX_ADDR_P0, dev->address, 5U);

    /* Dynamic payloads and dynamic payload length on pipe 0. */
    write_reg(dev, REG_DYNPD, 0x01U);

    /*
     * Enable FEATURE access. On nRF24L01+ devices where FEATURE
     * is locked, ACTIVATE 0x73 unlocks it. Check readback first
     * to avoid toggling access unnecessarily.
     */
    write_reg(dev, REG_FEATURE, FEATURE_EN_DPL);

    if ((read_reg(dev, REG_FEATURE) & FEATURE_EN_DPL) == 0U)
    {
        csn_low(dev);
        spi_byte(dev, CMD_ACTIVATE);
        spi_byte(dev, 0x73U);
        csn_high(dev);

        write_reg(dev, REG_FEATURE, FEATURE_EN_DPL);
    }

    if ((read_reg(dev, REG_FEATURE) & FEATURE_EN_DPL) == 0U)
    {
        return NRF24_ERROR;
    }

    clear_irqs(dev);
    NRF24_FlushRx(dev);
    NRF24_FlushTx(dev);

    /* Power up in standby mode, initially as a transmitter. */
    write_reg(dev, REG_CONFIG,
              CONFIG_EN_CRC | CONFIG_CRCO | CONFIG_PWR_UP);

    delay_ms(5U);

    return NRF24_SetTxMode(dev);
}

/* =========================================================
 * CONFIGURATION
 * ========================================================= */

NRF24_Status NRF24_SetChannel(NRF24 *dev, uint8_t channel)
{
    if (dev == NULL || channel > 125U)
    {
        return NRF24_INVALID_PARAM;
    }
  
    ce_low(dev);
    dev->channel = channel;
    write_reg(dev, REG_RF_CH, channel);

    return NRF24_OK;
}

NRF24_Status NRF24_SetDataRate(NRF24 *dev, uint8_t rate)
{
    uint8_t rf;

    if (dev == NULL || rate > 2U)
    {
        return NRF24_INVALID_PARAM;
    }
  
    ce_low(dev);

    rf = read_reg(dev, REG_RF_SETUP);
    rf &= (uint8_t)~((1U << 5) | (1U << 3));

    if (rate == 1U)       /* 2 Mbps */
    {
        rf |= (1U << 3);
    }
    else if (rate == 2U)  /* 250 kbps */
    {
        rf |= (1U << 5);
    }
  
    write_reg(dev, REG_RF_SETUP, rf);
    dev->data_rate = rate;

    return NRF24_OK;
}

NRF24_Status NRF24_SetTxPower(NRF24 *dev, uint8_t power)
{
    uint8_t rf;

    if (dev == NULL || power > 3U)
    {
        return NRF24_INVALID_PARAM;
    }
  
    ce_low(dev);

    rf = read_reg(dev, REG_RF_SETUP);
    rf &= (uint8_t)~(0x06U);
    rf |= (uint8_t)(power << 1);

    write_reg(dev, REG_RF_SETUP, rf);
    dev->tx_power = power;

    return NRF24_OK;
}

NRF24_Status NRF24_SetTxMode(NRF24 *dev)
{
    uint8_t config;

    if (dev == NULL)
    {
        return NRF24_INVALID_PARAM;
    }
  
    ce_low(dev);

    config = read_reg(dev, REG_CONFIG);
    config &= (uint8_t)~CONFIG_PRIM_RX;
    config |= CONFIG_PWR_UP | CONFIG_EN_CRC | CONFIG_CRCO;

    write_reg(dev, REG_CONFIG, config);
    delay_ms(2U);

    return NRF24_OK;
}

NRF24_Status NRF24_SetRxMode(NRF24 *dev)
{
    uint8_t config;

    if (dev == NULL)
    {
        return NRF24_INVALID_PARAM;
    }
  
    ce_low(dev);

    clear_irqs(dev);
    config = read_reg(dev, REG_CONFIG);
    config |= CONFIG_PRIM_RX | CONFIG_PWR_UP |
              CONFIG_EN_CRC | CONFIG_CRCO;

    write_reg(dev, REG_CONFIG, config);

    delay_ms(2U);
    ce_high(dev);

    return NRF24_OK;
}

/* =========================================================
 * TRANSMISSION
 * ========================================================= */

NRF24_Status NRF24_Send(NRF24 *dev,
                        const uint8_t *data,
                        uint8_t length,
                        uint32_t timeout_ms)
{
    uint32_t start;

    if (dev == NULL || data == NULL ||
        length == 0U || length > NRF24_MAX_PAYLOAD_SIZE)
    {
        return NRF24_INVALID_PARAM;
    }

    ce_low(dev);

    NRF24_SetTxMode(dev);
    clear_irqs(dev);
    NRF24_FlushTx(dev);

    /* Load the variable-length payload. */
    csn_low(dev);
    spi_byte(dev, CMD_W_TX_PAYLOAD);

    for (uint8_t i = 0; i < length; i++)
        spi_byte(dev, data[i]);

    csn_high(dev);

    /* A pulse of at least 10 us starts transmission. */
    ce_high(dev);
    delay_ms(1U);
    ce_low(dev);

    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint8_t status = NRF24_GetStatus(dev);

        if (status & STATUS_TX_DS)
        {
            clear_irqs(dev);
            return NRF24_OK;
        }

        if (status & STATUS_MAX_RT)
        {
            clear_irqs(dev);
            NRF24_FlushTx(dev);
            return NRF24_MAX_RETRIES;
        }
    }

    NRF24_FlushTx(dev);
    clear_irqs(dev);

    return NRF24_TIMEOUT;
}

bool NRF24_Available(NRF24 *dev)
{
    if (dev == NULL)
    {
        return false;
    }
  
    return (read_reg(dev, REG_FIFO_STATUS) & FIFO_RX_EMPTY) == 0U;
}

/* =========================================================
 * RECEPTION
 * ========================================================= */

NRF24_Status NRF24_Read(NRF24 *dev,
                        uint8_t *data,
                        uint8_t capacity,
                        uint8_t *length)
{
    uint8_t payload_len;

    if (dev == NULL || data == NULL || length == NULL ||
        capacity == 0U)
    {
        return NRF24_INVALID_PARAM;
    }

    *length = 0U;

    if (!NRF24_Available(dev))
    {
        return NRF24_ERROR;
    }
  
    /* Dynamic payload width is reported by the radio. */
    csn_low(dev);
    spi_byte(dev, CMD_R_RX_PL_WID);
    payload_len = spi_byte(dev, CMD_NOP);
    csn_high(dev);

    /* Widths greater than 32 indicate an invalid packet. */
    if (payload_len == 0U || payload_len > NRF24_MAX_PAYLOAD_SIZE)
    {
        NRF24_FlushRx(dev);
        clear_irqs(dev);
        return NRF24_ERROR;
    }

    if (payload_len > capacity)
    {
        /* Consume the packet to avoid leaving the FIFO blocked. */
        uint8_t discard[NRF24_MAX_PAYLOAD_SIZE];
        read_buf(dev, CMD_R_RX_PAYLOAD, discard, payload_len);
        clear_irqs(dev);
        return NRF24_INVALID_PARAM;
    }

    read_buf(dev, CMD_R_RX_PAYLOAD, data, payload_len);
    *length = payload_len;

    clear_irqs(dev);

    return NRF24_OK;
}

