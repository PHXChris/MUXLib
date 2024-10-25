// 1. Basic_Analog_4051.ino
// The simplest example of using an analog multiplexer
// Great for beginners to understand the basics

#include <MUXLib.h>
#include <AnalogMUX.h>

/* Connections:
   Arduino     74HC4051
   2    ---    S0 (Pin 11)
   3    ---    S1 (Pin 10)
   4    ---    S2 (Pin 9)
   A0   ---    Z  (Pin 3)
   GND  ---    GND, E (Pin 7, 6)
   5V   ---    VCC (Pin 16)
   
   Connect analog sensors (potentiometers, etc.) to channels Y0-Y7 (Pins 13,14,15,12,1,5,2,4)
*/

const uint8_t S0_PIN = 2;    // Channel select pin 0
const uint8_t S1_PIN = 3;    // Channel select pin 1
const uint8_t S2_PIN = 4;    // Channel select pin 2
const uint8_t SIG_PIN = A0;  // Signal pin (analog input)

uint8_t selectPins[] = {S0_PIN, S1_PIN, S2_PIN};
MUXLib::HC4051 mux(selectPins, SIG_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Basic 74HC4051 Analog Multiplexer Demo");
    
    mux.begin();  // Initialize the multiplexer
}

void loop() {
    // Read and print values from all 8 channels
    for (int channel = 0; channel < 8; channel++) {
        int value = mux.readChannel(channel);
        
        Serial.print("Channel ");
        Serial.print(channel);
        Serial.print(": ");
        Serial.println(value);
    }
    
    Serial.println("----------");
    delay(1000);
}