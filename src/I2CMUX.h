// I2C Multiplexer Module (I2CMUX.h)
#ifndef I2CMUX_H
#define I2CMUX_H

#include "MUXLib.h"

#if defined(ESP8266)
    #include <Wire.h>
    #define WIRE_IMPL TwoWire
#elif defined(ESP32)
    #include <Wire.h>
    #define WIRE_IMPL TwoWire
#else
    #include <Wire.h>
    #define WIRE_IMPL TwoWire
#endif

namespace MUXLib {
    // TCA9548A I2C Multiplexer
    class TCA9548A : public MUXManager {
    private:
        WIRE_IMPL* wire;
        bool scanning;
        uint32_t scanInterval;
        uint32_t lastScanTime;
        uint8_t scanStartCh;
        uint8_t scanEndCh;
        
    public:
        TCA9548A(uint8_t address = 0x70, WIRE_IMPL* wirePort = &Wire) 
            : MUXManager(address, 8), wire(wirePort), scanning(false),
              scanInterval(100), lastScanTime(0), scanStartCh(0), scanEndCh(7) {}
        
        MUXStatus begin() override {
            wire->begin();
            
            wire->beginTransmission(deviceAddress);
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_INIT;
            }
            
            enable();
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            wire->beginTransmission(deviceAddress);
            wire->write(1 << channel);
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_COMMUNICATION;
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        bool startScan(uint8_t startChannel = 0, uint8_t endChannel = 7) override {
            if (!isValidChannel(startChannel) || !isValidChannel(endChannel)) {
                return false;
            }
            scanning = true;
            scanStartCh = startChannel;
            scanEndCh = endChannel;
            currentChannel = startChannel - 1;  // Will increment to start on first update
            return true;
        }
        
        void stopScan() override {
            scanning = false;
        }
        
        void update() {
            if (scanning && (millis() - lastScanTime >= scanInterval)) {
                currentChannel++;
                if (currentChannel > scanEndCh || currentChannel < scanStartCh) {
                    currentChannel = scanStartCh;
                }
                setChannel(currentChannel);
                lastScanTime = millis();
            }
        }
        
        void setScanInterval(uint32_t interval) {
            scanInterval = interval;
        }

        // Platform specific I2C speed control
        void setI2CSpeed(uint32_t frequency) {
            #if defined(ESP8266) || defined(ESP32)
                wire->setClock(frequency);
            #else
                // Standard Arduino Wire library doesn't support dynamic clock speed
                // Could add platform specific implementations here
            #endif
        }
    };

    // PCA9547 I2C Multiplexer
    class PCA9547 : public MUXManager {
    private:
        WIRE_IMPL* wire;
        uint8_t resetPin;
        
    public:
        PCA9547(uint8_t address = 0x70, uint8_t rstPin = 255, WIRE_IMPL* wirePort = &Wire) 
            : MUXManager(address, 8), wire(wirePort), resetPin(rstPin) {}
            
        MUXStatus begin() override {
            wire->begin();
            
            if (resetPin != 255) {
                pinMode(resetPin, OUTPUT);
                digitalWrite(resetPin, HIGH);
            }
            
            wire->beginTransmission(deviceAddress);
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_INIT;
            }
            
            enable();
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            wire->beginTransmission(deviceAddress);
            wire->write(channel | 0x08); // Set enable bit
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_COMMUNICATION;
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        void reset() {
            if (resetPin != 255) {
                digitalWrite(resetPin, LOW);
                delayMicros(1);
                digitalWrite(resetPin, HIGH);
                delayMicros(1);
            }
        }

        // Platform specific I2C speed control
        void setI2CSpeed(uint32_t frequency) {
            #if defined(ESP8266) || defined(ESP32)
                wire->setClock(frequency);
            #endif
        }
    };

    // PCA9646 I2C Multiplexer with Voltage Translation
    class PCA9646 : public MUXManager {
    private:
        WIRE_IMPL* wire;
        uint8_t resetPin;
        uint8_t voltageLevel;  // Stored voltage level (for reference only)
        
    public:
        PCA9646(uint8_t address = 0x70, uint8_t rstPin = 255, WIRE_IMPL* wirePort = &Wire) 
            : MUXManager(address, 4), wire(wirePort), resetPin(rstPin), voltageLevel(33) {}
            
        MUXStatus begin() override {
            wire->begin();
            
            if (resetPin != 255) {
                pinMode(resetPin, OUTPUT);
                digitalWrite(resetPin, HIGH);
            }
            
            wire->beginTransmission(deviceAddress);
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_INIT;
            }
            
            enable();
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            wire->beginTransmission(deviceAddress);
            wire->write(1 << channel);
            if (wire->endTransmission() != 0) {
                return MUXStatus::ERROR_COMMUNICATION;
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        // Set voltage level (1.8V = 18, 2.5V = 25, 3.3V = 33, 5V = 50)
        void setVoltageLevel(uint8_t level) {
            voltageLevel = level;
            // Note: Actual voltage level is set by hardware pins
            // This is just for reference
        }

        void reset() {
            if (resetPin != 255) {
                digitalWrite(resetPin, LOW);
                delayMicros(1);
                digitalWrite(resetPin, HIGH);
                delayMicros(1);
            }
        }
    };
}

#endif