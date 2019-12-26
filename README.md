# smartcare-firmware

Firmware and tools necessary to build and flash SmartCare devices using nRF52832

## Setting up the enviroment to work with NRF52832 using Segger Embedded Studio
- Download the [nRF5 SDK version 16.0.0](https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK160098a08e2.zip).
- Follow [this](https://infocenter.nordicsemi.com/pdf/getting_started_ses.pdf) tutorial by Nordic.

## Setting up the enviroment to work with a standalone GCC
- Download the [nRF5 SDK version 16.0.0](https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK160098a08e2.zip).
- Install [J-Link Software and Documentation Pack (version 6.10g or later)](https://www.segger.com/downloads/)
- Install [nRF Command Line tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs)
- Download the recommended GCC (gcc-arm-none-eabi-7-2018-q2-update) from [this page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).
- Update the compiler path in `nRF5_SDK_16.0.0_98a08e2/components/toolchain/gcc/Makefile.[posix|windows]`
- Just go with `make` to compile and `make flash` to flash the device.

## Building the project
- Update the SDK variable to point to the SDK folder of your instalation in `ble_app_template/pca10040/s132/ses/ble_app_template_pca10040_s132.emProject`.
- Import the project into Segger Embedded Studio IDE.
- Press F7 to compile and then F5 to run in debug mode.
