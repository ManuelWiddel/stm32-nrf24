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
│   │   └
