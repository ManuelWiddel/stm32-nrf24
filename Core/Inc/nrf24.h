#ifndef NRF24_H
#define NRF24_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NRF24_MAX_PAYLOAD_SIZE 32U

typedef enum
{
    NRF24_OK = 0,
    NRF24_ERROR,
    NRF24_TIMEOUT,
    NRF24_MAX_RETRIES,
    NRF24_INVALID_PARAM
} NRF24_Status;

typedef struct
{
    SPI_HandleTypeDef *hspi;

    GPIO_TypeDef *csn_port;
    uint16_t csn_pin;

    GPIO_TypeDef *ce_port;
    uint16_t ce_pin;

    uint8_t address[5];
    uint8_t channel;
    uint8_t data_rate;       /* 0 = 1 Mbps, 1 = 2 Mbps, 2 = 250 kbps */
    uint8_t tx_power;        /* 0 = -18, 1 = -12, 2 = -6, 3 = 0 dBm */

    uint8_t status;
} NRF24;


/* =========================================================
 * PUBLIC API
 * ========================================================= */

NRF24_Status NRF24_Init(NRF24 *dev,
                        SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *csn_port, uint16_t csn_pin,
                        GPIO_TypeDef *ce_port, uint16_t ce_pin,
                        const uint8_t address[5]);

NRF24_Status NRF24_SetChannel(NRF24 *dev, uint8_t channel);

NRF24_Status NRF24_SetDataRate(NRF24 *dev, uint8_t rate);

NRF24_Status NRF24_SetTxPower(NRF24 *dev, uint8_t power);

NRF24_Status NRF24_SetTxMode(NRF24 *dev);

NRF24_Status NRF24_SetRxMode(NRF24 *dev);

NRF24_Status NRF24_Send(NRF24 *dev,
                        const uint8_t *data,
                        uint8_t length,
                        uint32_t timeout_ms);

bool NRF24_Available(NRF24 *dev);

NRF24_Status NRF24_Read(NRF24 *dev,
                        uint8_t *data,
                        uint8_t capacity,
                        uint8_t *length);

uint8_t NRF24_GetStatus(NRF24 *dev);

void NRF24_FlushRx(NRF24 *dev);

void NRF24_FlushTx(NRF24 *dev);

#ifdef __cplusplus
}
#endif

#endif
