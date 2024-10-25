// 1. Advanced_4067_SensorArray.ino
// Demonstrates reading multiple sensor types through one multiplexer
// and processing/analyzing the data

#include <MUXLib.h>
#include <AnalogMUX.h>

/* Sensor Setup:
   Channel 0-3:  Temperature sensors (analog)
   Channel 4-7:  Light sensors (analog)
   Channel 8-11: Potentiometers (analog)
   Channel 12-15: Buttons (digital)
*/

const uint8_t S0_PIN = 2;
const uint8_t S1_PIN = 3;
const uint8_t S2_PIN = 4;
const uint8_t S3_PIN = 5;
const uint8_t SIG_PIN = A0;

uint8_t selectPins[] = {S0_PIN, S1_PIN, S2_PIN, S3_PIN};
MUXLib::HC4067 mux(selectPins, SIG_PIN);

// Arrays to store sensor data
float temperatures[4];
int lightLevels[4];
int potValues[4];
bool buttonStates[4];

// Thresholds and calibration
const float TEMP_OFFSET = -50.0;  // Temperature sensor calibration
const float TEMP_SCALE = 0.488;   // Temperature conversion factor
const int LIGHT_THRESHOLD = 500;  // Light sensor threshold

void setup() {
    Serial.begin(9600);
    Serial.println("Advanced 74HC4067 Sensor Array");
    
    mux.begin();
    mux.setSettlingTime(50);  // Longer settling time for accuracy
}

void loop() {
    readAllSensors();
    processData();
    displayData();
    checkAlarms();
    delay(1000);
}

void readAllSensors() {
    // Read temperature sensors
    for (int i = 0; i < 4; i++) {
        int rawTemp = mux.readChannel(i);
        temperatures[i] = convertToTemperature(rawTemp);
    }
    
    // Read light sensors
    for (int i = 0; i < 4; i++) {
        lightLevels[i] = mux.readChannel(i + 4);
    }
    
    // Read potentiometers
    for (int i = 0; i < 4; i++) {
        potValues[i] = mux.readChannel(i + 8);
    }
    
    // Read buttons
    for (int i = 0; i < 4; i++) {
        mux.setChannel(i + 12);
        delay(1);  // Settling time for digital
        buttonStates[i] = (analogRead(SIG_PIN) < 100);  // Active low
    }
}

float convertToTemperature(int rawValue) {
    float voltage = (rawValue * 5.0) / 1024.0;
    return (voltage * 100.0) + TEMP_OFFSET;
}

void processData() {
    float avgTemp = 0;
    int avgLight = 0;
    
    // Calculate averages
    for (int i = 0; i < 4; i++) {
        avgTemp += temperatures[i];
        avgLight += lightLevels[i];
    }
    avgTemp /= 4.0;
    avgLight /= 4;
    
    // Store results (could be expanded to SD card, etc.)
    Serial.print("Average Temperature: ");
    Serial.print(avgTemp);
    Serial.println(" C");
    
    Serial.print("Average Light Level: ");
    Serial.println(avgLight);
}

void displayData() {
    Serial.println("\n=== Sensor Readings ===");
    
    // Temperature readings
    Serial.println("Temperatures:");
    for (int i = 0; i < 4; i++) {
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(temperatures[i]);
        Serial.println(" C");
    }
    
    // Light levels
    Serial.println("\nLight Levels:");
    for (int i = 0; i < 4; i++) {
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(lightLevels[i]);
    }
    
    // Potentiometer positions
    Serial.println("\nPotentiometer Positions:");
    for (int i = 0; i < 4; i++) {
        Serial.print("Pot ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(map(potValues[i], 0, 1023, 0, 100));
        Serial.println("%");
    }
    
    // Button states
    Serial.println("\nButton States:");
    for (int i = 0; i < 4; i++) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(buttonStates[i] ? "PRESSED" : "Released");
    }
}

void checkAlarms() {
    // Check temperature thresholds
    for (int i = 0; i < 4; i++) {
        if (temperatures[i] > 50.0) {  // High temperature alarm
            Serial.print("WARNING: High temperature on sensor ");
            Serial.println(i);
        }
    }
    
    // Check light levels
    for (int i = 0; i < 4; i++) {
        if (lightLevels[i] > LIGHT_THRESHOLD) {
            Serial.print("Notice: High light level on sensor ");
            Serial.println(i);
        }
    }
}