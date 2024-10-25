// 2. Advanced_I2C_SensorHub.ino
// Advanced I2C multiplexer example showing how to manage multiple
// identical I2C devices (like BME280 sensors) on different channels

#include <Wire.h>
#include <MUXLib.h>
#include <I2CMUX.h>
// Note: You would also need to include your sensor library
// #include <BME280.h>  // Example sensor library

const uint8_t NUM_SENSORS = 4;  // Number of connected sensors
const uint32_t UPDATE_INTERVAL = 60000;  // Update every minute

MUXLib::TCA9548A i2cMux;
// BME280 sensors[NUM_SENSORS];  // Array of sensor objects

struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    bool valid;
    unsigned long lastUpdate;
} sensorData[NUM_SENSORS];

void setup() {
    Serial.begin(9600);
    Serial.println("Advanced I2C Sensor Hub");
    
    Wire.begin();
    
    if (i2cMux.begin() != MUXLib::MUXStatus::OK) {
        Serial.println("Failed to initialize I2C multiplexer!");
        while(1);
    }
    
    // Initialize sensors on each channel
    initializeSensors();
}

void loop() {
    static unsigned long lastUpdate = 0;
    
    // Update sensor readings every minute
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        updateAllSensors();
        displayResults();
        lastUpdate = millis();
    }
    
    // Check for serial commands
    checkCommands();
}

void initializeSensors() {
    for (uint8_t channel = 0; channel < NUM_SENSORS; channel++) {
        i2cMux.setChannel(channel);
        delay(10);
        
        /* Example sensor initialization
        if (!sensors[channel].begin()) {
            Serial.print("Failed to initialize sensor on channel ");
            Serial.println(channel);
            sensorData[channel].valid = false;
        } else {
            sensorData[channel].valid = true;
        }
        */
    }
}

void updateAllSensors() {
    for (uint8_t channel = 0; channel < NUM_SENSORS; channel++) {
        if (sensorData[channel].valid) {
            i2cMux.setChannel(channel);
            delay(10);
            
            /* Example sensor reading
            sensorData[channel].temperature = sensors[channel].readTemperature();
            sensorData[channel].humidity = sensors[channel].readHumidity();
            sensorData[channel].pressure = sensors[channel].readPressure() / 100.0F;
            */
            sensorData[channel].lastUpdate = millis();
        }
    }
}

void displayResults() {
    Serial.println("\n=== Sensor Readings ===");
    
    for (uint8_t channel = 0; channel < NUM_SENSORS; channel++) {
        Serial.print("Channel ");
        Serial.print(channel);
        Serial.print(": ");
        
        if (sensorData[channel].valid) {
            Serial.print("Temp: ");
            Serial.print(sensorData[channel].temperature);
            Serial.print("°C, Humidity: ");
            Serial.print(sensorData[channel].humidity);
            Serial.print("%, Pressure: ");
            Serial.print(sensorData[channel].pressure);
            Serial.println(" hPa");
        } else {
            Serial.println("SENSOR NOT AVAILABLE");
        }
    }
}

void checkCommands() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'r':  // Force refresh
                updateAllSensors();
                displayResults();
                break;
            
            case 's':  // Scan I2C devices
                scanI2CDevices();
                break;
            
            case 'h':  // Show help
                showHelp();
                break;
        }
    }
}

void scanI2CDevices() {
    Serial.println("\nScanning I2C devices on all channels...");
    
    for (uint8_t channel = 0; channel < 8; channel++) {
        Serial.print("\nChannel ");
        Serial.print(channel);
        Serial.println(":");
        
        i2cMux.setChannel(channel);
        delay(10);
        
        for (uint8_t addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            uint8_t error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print("Device found at address 0x");
                if (addr < 16) Serial.print("0");
                Serial.println(addr, HEX);
            }
        }
    }
}

void showHelp() {
    Serial.println("\nAvailable Commands:");
    Serial.println("r - Refresh all sensor readings");
    Serial.println("s - Scan for I2C devices");
    Serial.println("h - Show this help message");
}