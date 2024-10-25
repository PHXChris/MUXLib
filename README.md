# MUXLib - Arduino Multiplexer Library

A beginner-friendly Arduino library for controlling a wide selection of analog and digital multiplexers. This library makes it easy to use multiplexers (MUX) to expand the number of inputs/outputs on your Arduino.

## Supported Devices

### Analog Multiplexers
- 74HC4051 (8-channel)
- 74HC4052 (Dual 4-channel)
- 74HC4053 (Triple 2-channel)
- 74HC4067 (16-channel)
- ADG508A/ADG509A
- ADG706/ADG707
- ADG506A/ADG507A
- MPC506A/MPC507A
- DG408/DG409
- MAX4051A
- MAX4582

### Digital Multiplexers
- CD74HC4067 (16-channel)
- CD74HC4051 (8-channel)

### I²C Multiplexers
- TCA9548A (8-channel)

## Installation

### Using the Arduino Library Manager
1. Open the Arduino IDE
2. Go to `Sketch > Include Library > Manage Libraries...`
3. Search for "MUXLib"
4. Click Install

### Manual Installation
1. Download the ZIP file from this repository
2. Open the Arduino IDE
3. Go to `Sketch > Include Library > Add .ZIP Library...`
4. Select the downloaded ZIP file

## Quick Start

## Including Header Files

The library is modular to keep code size minimal. Include only the headers you need for your specific multiplexer:

### Basic Include (Required)
```cpp
#include <MUXLib.h>  // Always include this base header
```

### Analog Multiplexers
```cpp
#include <AnalogMUX.h>  // For these multiplexers:
// - 74HC4051 (8-channel)
// - 74HC4067 (16-channel)
// - 74HC4052 (Dual 4-channel)
// - 74HC4053 (Triple 2-channel)
// - ADG508A/ADG509A
// - ADG706/ADG707
// - ADG506A/ADG507A
// - MPC506A/MPC507A
// - DG408/DG409
// - MAX4051A
// - MAX4582
```

### Digital Multiplexers
```cpp
#include <DigitalMUX.h>  // For these multiplexers:
// - CD74HC4067
// - CD74HC4051
```

### I²C Multiplexers
```cpp
#include <I2CMUX.h>  // For these multiplexers:
// - TCA9548A
```

### Examples

1. Using a 74HC4051 analog multiplexer:
```cpp
#include <MUXLib.h>
#include <AnalogMUX.h>

MUXLib::HC4051 mux(selectPins, sigPin);
```

2. Using a CD74HC4067 for digital inputs:
```cpp
#include <MUXLib.h>
#include <DigitalMUX.h>

MUXLib::CD74HC4067 mux(selectPins, sigPin);
```

3. Using a TCA9548A I²C multiplexer:
```cpp
#include <MUXLib.h>
#include <I2CMUX.h>

MUXLib::TCA9548A i2cMux;
```

4. Using multiple types of multiplexers:
```cpp
#include <MUXLib.h>
#include <AnalogMUX.h>
#include <DigitalMUX.h>
#include <I2CMUX.h>

MUXLib::HC4051 analogMux(analogPins, analogSigPin);
MUXLib::CD74HC4067 digitalMux(digitalPins, digitalSigPin);
MUXLib::TCA9548A i2cMux;
```

### Important Notes
- Always include `MUXLib.h` first
- Include only the headers for the multiplexer types you're using
- Including unnecessary headers will increase program size
- All classes are in the `MUXLib` namespace

Would you like me to:
1. Add more examples of specific combinations?
2. Include information about header dependencies?
3. Add a troubleshooting section for include-related issues?
4. Add information about platform-specific includes?

### Basic Example - 74HC4051 (8-channel analog multiplexer)
```cpp
#include <MUXLib.h>
#include <AnalogMUX.h>

// Define pins
const uint8_t S0_PIN = 2;    // Connect to S0 on 4051
const uint8_t S1_PIN = 3;    // Connect to S1 on 4051
const uint8_t S2_PIN = 4;    // Connect to S2 on 4051
const uint8_t SIG_PIN = A0;  // Connect to Z on 4051

// Create multiplexer instance
uint8_t selectPins[] = {S0_PIN, S1_PIN, S2_PIN};
MUXLib::HC4051 mux(selectPins, SIG_PIN);

void setup() {
    Serial.begin(9600);
    mux.begin();
}

void loop() {
    // Read all 8 channels
    for (int channel = 0; channel < 8; channel++) {
        int value = mux.readChannel(channel);
        Serial.print("Channel ");
        Serial.print(channel);
        Serial.print(": ");
        Serial.println(value);
    }
    delay(1000);
}
```

## Wiring Examples

### 74HC4051 Connections
```
Arduino     74HC4051
2    ---    S0 (Pin 11)
3    ---    S1 (Pin 10)
4    ---    S2 (Pin 9)
A0   ---    Z  (Pin 3)
GND  ---    GND, E (Pin 7, 6)
5V   ---    VCC (Pin 16)
```

### 74HC4067 Connections
```
Arduino     74HC4067
2    ---    S0 (Pin 10)
3    ---    S1 (Pin 11)
4    ---    S2 (Pin 14)
5    ---    S3 (Pin 13)
12   ---    Z  (Pin 1)
GND  ---    GND, E (Pin 7, 15)
5V   ---    VCC (Pin 24)
```

## Features

- Easy-to-use interface for beginners
- Support for multiple multiplexer types
- Configurable settling time for accurate readings
- Differential reading support (where applicable)
- Break-before-make switching
- Error checking and status reporting
- Channel scanning functionality
- Interrupt support (where applicable)

## Class Reference

### Common Methods
- `begin()` - Initialize the multiplexer
- `setChannel(channel)` - Select a specific channel
- `readChannel(channel)` - Read value from a specific channel
- `enable()` - Enable the multiplexer
- `disable()` - Disable the multiplexer
- `setSettlingTime(microseconds)` - Set analog settling time

### Status Codes
```cpp
enum class MUXStatus {
    OK,
    ERROR_INIT,
    ERROR_COMMUNICATION,
    ERROR_CHANNEL_INVALID,
    ERROR_NOT_ENABLED,
    ERROR_OVERFLOW
};
```

## Examples

The library includes several examples demonstrating different use cases:

### Basic Examples
- `Basic_4051_Analog.ino` - Reading analog sensors
- `Basic_4067_Buttons.ino` - Reading multiple buttons
- `Basic_I2C_TCA9548.ino` - Managing multiple I²C devices
- `Basic_Differential_ADG409.ino` - Differential readings
- `Basic_Test.ino` - Simple test program

### Advanced Examples
- `Advanced_4067_SensorArray.ino` - Multiple sensor types and processing
- `Advanced_I2C_SensorHub.ino` - Advanced I²C device management

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This library is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

If you find a bug or want to request a new feature, please open an issue.

## Credits

- Library developed by Chris Formeister
- Thanks to the Arduino community for testing and feedback

## Version History

- 1.0.0 (2024-02-15)
  - Initial release
  - Support for basic multiplexer types
  - Basic and advanced examples
