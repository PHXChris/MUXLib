// 3. Basic_I2C_TCA9548.ino
// Basic example of using an I2C multiplexer to read multiple
// I2C devices of the same address

#include <Wire.h>
#include <MUXLib.h>
#include <I2CMUX.h>

/* Connections:
   Arduino     TCA9548A
   SDA   ---   SDA
   SCL   ---   SCL
   GND   ---   GND, A0, A1, A2
   5V    ---   VCC
   
   Connect I2C devices to the SC0/SD0 through SC7/SD7 pairs
*/

MUXLib::TCA9548A i2cMux;  // Default address 0x70

void setup() {
    Serial.begin(9600);
    Serial.println("Basic TCA9548A I2C Multiplexer Demo");
    
    Wire.begin();
    
    if (i2cMux.begin() != MUXLib::MUXStatus::OK) {
        Serial.println("Multiplexer not found!");
        while(1);
    }
}

void loop() {
    // Scan each channel for I2C devices
    for (int channel = 0; channel < 8; channel++) {
        Serial.print("Scanning channel ");
        Serial.println(channel);
        
        i2cMux.setChannel(channel);
        
        // Scan for I2C devices on current channel
        for (byte addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            byte error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print("I2C device found at 0x");
                if (addr < 16) Serial.print("0");
                Serial.println(addr, HEX);
            }
        }
    }
    
    Serial.println("----------");
    delay(5000);  // Scan every 5 seconds
}

// 4. Basic_Differential_ADG409.ino
// Basic example of using a differential multiplexer
// to read voltage differences between pairs of inputs

#include <MUXLib.h>
#include <AnalogMUX.h>

/* Connections:
   Arduino     ADG409
   2    ---    A0 (Pin 1)
   3    ---    A1 (Pin 16)
   4    ---    A2 (Pin 15)
   A0   ---    D+ (Pin 4)
   A1   ---    D- (Pin 5)
   GND  ---    GND, EN (Pin 8, 14)
   5V   ---    VDD (Pin 9)
   
   Connect differential voltage sources to channel pairs
*/

const uint8_t S0_PIN = 2;
const uint8_t S1_PIN = 3;
const uint8_t S2_PIN = 4;
const uint8_t POS_PIN = A0;
const uint8_t NEG_PIN = A1;

uint8_t selectPins[] = {S0_PIN, S1_PIN, S2_PIN};
MUXLib::DG408 mux(selectPins, POS_PIN, NEG_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Basic ADG409 Differential Multiplexer Demo");
    
    mux.begin();
}

void loop() {
    // Read differential voltages from all channels
    for (int channel = 0; channel < 8; channel++) {
        int diffValue = mux.readDifferential(channel);
        
        Serial.print("Channel pair ");
        Serial.print(channel);
        Serial.print(" difference: ");
        Serial.println(diffValue);
    }
    
    Serial.println("----------");
    delay(1000);
}