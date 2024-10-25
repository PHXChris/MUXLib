// Basic_Test.ino
// A simple test sketch to verify your multiplexer is working
// Works with any multiplexer type

/* Connections:
   Connect LEDs or voltmeter to outputs to verify channel switching
   Connect a potentiometer to one channel to test reading values
*/

#include <MUXLib.h>
#include <AnalogMUX.h>

// Change these for your multiplexer type
const uint8_t NUM_CHANNELS = 8;  // 8 for 4051, 16 for 4067, etc.
const uint8_t NUM_SELECT_PINS = 3;  // 3 for 4051, 4 for 4067, etc.

// Define pins - change these to match your wiring
const uint8_t selectPins[] = {2, 3, 4};  // Add more pins if needed
const uint8_t SIG_PIN = A0;

// Create multiplexer object - change class to match your multiplexer
MUXLib::HC4051 mux(selectPins, SIG_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Multiplexer Test Program");
    Serial.println("Enter channel number (0-7) to select");
    Serial.println("Enter 'r' to read current channel");
    Serial.println("Enter 's' to scan all channels");
    
    mux.begin();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        if (cmd >= '0' && cmd <= '7') {
            int channel = cmd - '0';
            mux.setChannel(channel);
            Serial.print("Selected channel ");
            Serial.println(channel);
        }
        else if (cmd == 'r') {
            int value = mux.readChannel(mux.getChannel());
            Serial.print("Channel ");
            Serial.print(mux.getChannel());
            Serial.print(" value: ");
            Serial.println(value);
        }
        else if (cmd == 's') {
            Serial.println("Scanning all channels:");
            for (int i = 0; i < NUM_CHANNELS; i++) {
                int value = mux.readChannel(i);
                Serial.print("Channel ");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(value);
            }
        }
    }
}