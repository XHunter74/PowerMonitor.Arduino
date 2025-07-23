# PowerMonitor Arduino

A real-time power monitoring system using Arduino and PZEM-004T v3.0 sensor for measuring electrical parameters like voltage, current, and frequency.

## Features

- **Real-time Monitoring**: Measures voltage, current, and frequency every second
- **High Precision**: 3-decimal place accuracy for all measurements
- **JSON Output**: Structured data output via serial communication
- **Calibration Support**: Adjustable calibration coefficients for accurate readings
- **Multi-Board Support**: Compatible with Arduino Mega 2560 and Arduino Nano
- **LED Status Indicator**: Visual feedback with blinking LED
- **Serial Commands**: Interactive control via serial interface
- **Error Handling**: Robust sensor reading validation and fallback values

## Hardware Requirements

- Arduino Mega 2560 or Arduino Nano
- PZEM-004T v3.0 power meter sensor
- Connecting wires
- LED (built-in pin 13)

## Wiring

### PZEM-004T Connections
- **PZEM RX** ↔ **Arduino Pin 11** (PZEM_TX_PIN)
- **PZEM TX** ↔ **Arduino Pin 10** (PZEM_RX_PIN)
- **PZEM VCC** ↔ **Arduino 5V**
- **PZEM GND** ↔ **Arduino GND**

### LED Indicator
- Built-in LED on Pin 13 (automatically configured)

## Software Dependencies

- **PlatformIO** development environment
- **PZEM-004T-v30** library (v1.1.2+)
- **Arduino Framework**

## Installation

1. Clone this repository
2. Open the project in PlatformIO
3. Build and upload to your Arduino board

```bash
# Using PlatformIO CLI
platformio run --target upload
```

## Serial Commands

The device accepts the following serial commands at 57600 baud rate:

| Command | Description | Example |
|---------|-------------|---------|
| `i` | Display current calibration coefficients | `i` |
| `v<value>` | Set voltage calibration coefficient | `v0.92` |
| `c<value>` | Set current calibration coefficient | `c2.52` |
| `p<value>` | Set power factor calibration coefficient | `p1.0` |
| `s<v>:<c>:<p>` | Set all coefficients at once | `s0.92:2.52:1.0` |
| `d` | Display device info (version, build date) | `d` |
| `w` | Toggle pause/resume monitoring | `w` |
| `r` | Reset the device | `r` |
| `h` | Display help information | `h` |

## JSON Output Format

### Data Output
```json
{
  "type": "data",
  "voltage": 230.123,
  "current": 5.456,
  "frequency": 50.000
}
```

### Coefficients Output
```json
{
  "type": "coefficients",
  "voltage": 0.920,
  "current": 2.520,
  "powerFactor": 1.000
}
```

### Device Info Output
```json
{
  "type": "info",
  "version": "1.0.0",
  "date": "2025-07-22"
}
```

## Configuration

### Default Settings
- **Voltage Calibration**: 0.92
- **Current Calibration**: 2.52
- **Power Factor Calibration**: 1.0
- **Maximum Voltage**: 400.0V
- **Maximum Current**: 100.0A
- **Serial Baud Rate**: 57600
- **Measurement Interval**: 999ms (approximately 1 second)

### Pin Configuration
Default pins can be overridden by defining `PZEM_RX_PIN` and `PZEM_TX_PIN`:

```cpp
#define PZEM_RX_PIN 10
#define PZEM_TX_PIN 11
```

## Project Structure

```
PowerMonitor.Arduino/
├── src/
│   ├── main.cpp           # Main application logic
│   └── led_control.cpp    # LED control functions
├── include/
│   ├── led_control.h      # LED control header
│   └── version.h          # Version definitions
├── platformio.ini         # PlatformIO configuration
├── buildscript_versioning.py  # Build version script
└── README.md             # This file
```

## Build Environments

The project supports two build environments:

- **ArduinoMega2560**: For Arduino Mega 2560 boards
- **ArduinoNano**: For Arduino Nano boards

Select the appropriate environment in PlatformIO or specify it via CLI:

```bash
# Build for Arduino Mega
platformio run -e ArduinoMega2560

# Build for Arduino Nano  
platformio run -e ArduinoNano
```

## Error Handling

- **Sensor Validation**: Automatically detects invalid sensor readings (NaN values)
- **Fallback Values**: Uses previous valid readings when sensor errors occur
- **Range Validation**: Filters out readings outside safe operating ranges
- **Error Counting**: Tracks consecutive voltage reading errors

## Calibration

The system supports calibration to improve measurement accuracy:

1. **Voltage Calibration**: Compensates for voltage divider accuracy
2. **Current Calibration**: Adjusts for current transformer variations
3. **Power Factor Calibration**: Fine-tunes power factor calculations

Calibration values can be set via serial commands and are applied in real-time.

## License

This project is open source. Please check the repository for license details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For questions and support, please open an issue on the GitHub repository.
