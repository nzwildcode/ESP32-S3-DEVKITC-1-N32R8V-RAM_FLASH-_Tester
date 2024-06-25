# ESP32-S3-DEVKITC-1-N32R8V-RAM_FLASH _Tester

## Purpose

This program provides detailed information about the ESP32-S3 chip and tests its RAM and flash memory capacities. It's designed to help developers understand the characteristics of their ESP32-S3 module and verify its memory capabilities.

## Features

- Displays comprehensive chip information:
  - Model
  - Number of cores
  - Revision
  - CPU frequency
  - MAC address
  - PSRAM size and usage
  - Built-in RAM size and usage
  - Partition information
- Tests RAM capacity by allocating as much PSRAM as possible
- Tests flash memory capacity by writing data to the LittleFS filesystem
- Provides a simple command interface for running tests and viewing results

## How to Use

1. Upload the program to your ESP32-S3 board using PlatformIO or the Arduino IDE.
2. Open the Serial Monitor at 115200 baud.
3. The program will print detailed chip and memory information on startup.
4. Use the following commands in the Serial Monitor:
   - Type `w` and press enter to start the memory capacity tests (RAM and flash).
   - Type `r` and press enter to view the results of the last memory test.

## Expected Results

- On startup: Detailed chip information is displayed
- When `w` is entered: The program attempts to fill RAM and flash to determine their capacities
- When `r` is entered: Results of the last memory test are displayed

## Limitations

- This program tests the ability to allocate memory and write to flash, which may not represent the full capacity of the module due to system overhead and reserved areas.
- The flash memory test uses the LittleFS filesystem, so the reported capacity will be less than the total flash size of the chip.
- This tool is for informational purposes and should not be used as a definitive measure of module specifications.

## Tested Hardware

- ESP32-S3-DevKitC-1

## Contributing

Contributions are welcome! Please open a pull request with your proposed changes or improvements.

## License

This project is licensed under the MIT License.

## Acknowledgements

Special thanks to Boris Jäger (sivar2311) from the PlatformIO community for guidance on the correct platform.ini configurations.
https://github.com/sivar2311/ESP32-S3-PlatformIO-Flash-and-PSRAM-configurations