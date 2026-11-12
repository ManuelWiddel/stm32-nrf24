# stm32-nrf24

STM32 HAL driver for the Nordic nRF24L01+ 2.4 GHz wireless transceiver.

## Project Overview

This project provides a lightweight driver for the nRF24L01+ wireless transceiver using STM32 HAL SPI.

The implementation supports:

- Transmit mode
- Receive mode
- Dynamic payload length
- Auto acknowledgment
- Automatic retransmission
- Runtime channel configuration
- Runtime data rate configuration
- Runtime power configuration

The driver is designed for STM32CubeIDE projects and relies only on STM32 HAL.

## Key Features

- STM32 HAL SPI interface
- Dynamic payload support
- Auto acknowledgment
- Automatic retransmission
- Configurable RF channel
- Configurable data rate
- Configurable TX power
- Timeout-protected transmissions
- FIFO management functions
- Small memory footprint

## Hardware

| Component | Specification |
|------------|------------|
| Radio | nRF24L01+ |
| Interface | SPI |
| Frequency Range | 2.4 GHz |
| Payload Size | 1-32 bytes |
| MCU | STM32 with HAL SPI support |

## Software

- Programming Language: C
- IDE: STM32CubeIDE / STM32CubeMX
- Framework: STM32 HAL
- Communication: SPI

## Repository Structure

```text
stm32-nrf24/
├── Core/
│   ├── Inc/
│   │   └── nrf24.h
│   └── Src/
│       └── nrf24.c
├── LICENSE
└── README.md
```

## Wiring Example

| nRF24L01+ | STM32 |
|------------|------------|
| VCC | 3.3 V |
| GND | GND |
| CE | GPIO |
| CSN | GPIO |
| SCK | SPI SCK |
| MOSI | SPI MOSI |
| MISO | SPI MISO |

## Initialization

```c
NRF24 radio;

uint8_t address[5] =
{
    'N',
    'O',
    'D',
    'E',
    '1'
};

NRF24_Init(&radio,
           &hspi1,
           NRF_CSN_GPIO_Port,
           NRF_CSN_Pin,
           NRF_CE_GPIO_Port,
           NRF_CE_Pin,
           address);
```

## Transmit Example

```c
uint8_t tx_data[] =
{
    1,
    2,
    3,
    4
};

NRF24_Status status =
    NRF24_Send(&radio,
               tx_data,
               sizeof(tx_data),
               100);
```

## Receive Example

```c
uint8_t buffer[32];
uint8_t length;

if (NRF24_Available(&radio))
{
    NRF24_Read(&radio,
               buffer,
               sizeof(buffer),
               &length);
}
```

## Set RF Channel

```c
NRF24_SetChannel(&radio, 76);
```

Valid range:

```text
0 - 125
```

## Set Data Rate

```c
NRF24_SetDataRate(&radio, 0);
```

| Value | Rate |
|---------|---------|
| 0 | 1 Mbps |
| 1 | 2 Mbps |
| 2 | 250 kbps |

## Set TX Power

```c
NRF24_SetTxPower(&radio, 3);
```

| Value | Power |
|---------|---------|
| 0 | -18 dBm |
| 1 | -12 dBm |
| 2 | -6 dBm |
| 3 | 0 dBm |

## Mode Switching

```c
NRF24_SetTxMode(&radio);
```

```c
NRF24_SetRxMode(&radio);
```

## Status Codes

| Status | Description |
|----------|----------|
| NRF24_OK | Operation successful |
| NRF24_ERROR | General error |
| NRF24_TIMEOUT | Transmission timeout |
| NRF24_MAX_RETRIES | Maximum retransmissions reached |
| NRF24_INVALID_PARAM | Invalid function parameter |

## Notes

- Designed for nRF24L01+
- Uses dynamic payload length
- Uses auto acknowledgment
- Uses automatic retransmission
- Supports payloads from 1 to 32 bytes
- Requires a stable 3.3 V supply

## Future Improvements

- Example application project

## License

