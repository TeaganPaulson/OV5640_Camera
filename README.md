# OV5640 Camera Project

This ESP-IDF project demonstrates how to interface with the OV5640 camera module on an ESP32 microcontroller. The OV5640 is a 5-megapixel CMOS image sensor capable of capturing high-resolution images and videos, commonly used in embedded vision applications.

The project initializes the camera, configures it for basic image capture, and provides examples for streaming video or taking snapshots. It leverages the ESP32's I2C and SPI interfaces for communication with the camera module.

## Prerequisites

- ESP32 development board (e.g., ESP32-WROOM-32)
- OV5640 camera module
- ESP-IDF toolchain installed (version 4.4 or later recommended)
- Visual Studio Code with ESP-IDF extension (optional, for development)

## How to Build and Run

1. Clone or copy this project to your workspace.
2. Open the project in VS Code with ESP-IDF extension.
3. Configure the project: `idf.py menuconfig` (set camera pins, resolution, etc.)
4. Build the project: `idf.py build`
5. Flash to ESP32: `idf.py flash`
6. Monitor output: `idf.py monitor`

For more details on ESP-IDF setup, refer to the [official documentation](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html).

## Example Usage

The main application in `main.c` initializes the camera and starts capturing frames. You can modify the code to integrate with Wi-Fi for streaming or save images to SD card.

## Project Structure

```
├── CMakeLists.txt          # Main project CMake configuration
├── main
│   ├── CMakeLists.txt      # Component CMake configuration
│   └── main.c              # Main application code
├── components              # Custom components (if any)
│   └── camera              # Camera driver component
└── README.md               # This file
```

- `main.c`: Contains the application logic for camera initialization and capture.
- `components/camera/`: Includes drivers for OV5640 (based on ESP-IDF camera components).

## Configuration

Use `idf.py menuconfig` to configure:
- Camera model: OV5640
- Pin assignments for I2C/SPI
- Image resolution (e.g., 640x480, 1280x720)
- Frame rate and JPEG quality

## Troubleshooting

- Ensure proper wiring: Connect OV5640 to ESP32 pins as per datasheet.
- Check power supply: OV5640 requires stable 3.3V.
- For issues, refer to ESP-IDF camera examples or community forums.

## Contributing

Feel free to submit issues or pull requests for improvements.

## License

This project is licensed under the MIT License.