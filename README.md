# LoRa MAC class A node on Nordic nRF52

## Hardware

Developed and tested on [Nordic's PCA10040 DK](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52-DK)
and [Semtech's SX1276MB1MAS](https://developer.mbed.org/components/SX1276MB1xAS/)

###### Modifications

Cut SB7 (LED3) and SB8 (LED4) - used for DIO4&5

## Software

Built using Nordic's SDK v13.0.0 and gcc version 5.4.1
Using S132 Softdevice v4.0.2

Debug prints using Segger's RTT mechanism.

## Configuration

Set `LORAWAN_APPLICATION_EUI` and `LORAWAN_APPLICATION_KEY` in `config/Comissioning.h`
Device EUI is taken from nRF unique device ID (FICR DEVICEADDR).

## Building

Modify `SDK_ROOT` in `Makefile` to point to Nordic's SDK root
Make sure nrfjprog is in `PATH`
run `make flash_softdevice && make flash`

