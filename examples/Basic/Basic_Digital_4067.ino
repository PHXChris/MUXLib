// 2. Basic_Digital_4067.ino
// Basic example of reading multiple buttons or switches
// Using a digital multiplexer

#include <MUXLib.h>
#include <DigitalMUX.h>

/* Connections:
   Arduino     74HC4067
   2    ---    S0 (Pin 10)
   3    ---    S1 (Pin 11)
   4    ---    S2 (Pin 14)
   5    ---    S3 (Pin 13)
   12   ---    Z  (Pin 1)
   GND  ---    GND, E (Pin 7, 15)
   5V   ---    VCC (Pin 24)
   
   Connect buttons between GND and channels Y0-Y15
   (Pins 9,8,7,6,5,4,3,2,23,22,21,20,19,18,17,16)
*/

const uint8_t S0_PIN = 2;
const uint8_t S1_PIN = 3;
const uint8_t S2_PIN = 4;
const uint8_t S3_PIN = 5;
const uint8_t SIG_PIN = 12;

uint8_t selectPins[] = {S0_PIN, S1_PIN, S2_PIN, S3_PIN};
MUXLib::CD74HC4067 mux(selectPins, SIG_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Basic 74HC4067 Digital Multiplexer Demo");
    
    mux.begin();
    pinMode(SIG_PIN, INPUT_PULLUP);  // Enable pull-up for buttons
}

void loop() {
    // Check all 16 channels for button presses
    for (int channel = 0; channel < 16; channel++) {
        mux.setChannel(channel);
        delay(1);  // Brief delay for stability
        
        if (digitalRead(SIG_PIN) == LOW) {  // Button is pressed
            Serial.print("Button ");
            Serial.print(channel);
            Serial.println(" pressed");
        }
    }
    delay(100);
}