// Core MUX Library Implementation (MUXLib.cpp)
#include "MUXLib.h"

namespace MUXLib {
    MUXManager::MUXManager(uint8_t address, uint8_t channels) 
        : deviceAddress(address)
        , enabled(false)
        , currentChannel(0)
        , maxChannels(channels)
        , interruptHandler(nullptr)
        , interruptFlag(false)
        , interruptPin(255) {
    }

    MUXManager::~MUXManager() {
        if (interruptPin != 255) {
            detachInterrupt();
        }
    }

    uint8_t MUXManager::getChannel() {
        return currentChannel;
    }

    bool MUXManager::isEnabled() {
        return enabled;
    }

    void MUXManager::enable() {
        enabled = true;
    }

    void MUXManager::disable() {
        enabled = false;
    }

    void MUXManager::attachInterrupt(InterruptCallback callback, uint8_t pin, InterruptMode mode) {
        interruptHandler = callback;
        interruptPin = pin;
        
        if (pin != 255) {
            pinMode(pin, INPUT_PULLUP);
            uint8_t arduinoMode;
            
            switch (mode) {
                case InterruptMode::LOW_LEVEL:
                    arduinoMode = LOW;
                    break;
                case InterruptMode::HIGH_LEVEL:
                    arduinoMode = HIGH;
                    break;
                case InterruptMode::FALLING_EDGE:
                    arduinoMode = FALLING;
                    break;
                case InterruptMode::RISING_EDGE:
                    arduinoMode = RISING;
                    break;
                case InterruptMode::CHANGE:
                default:
                    arduinoMode = CHANGE;
                    break;
            }

            // Store this pointer for ISR
            static MUXManager* instance = this;
            
            ::attachInterrupt(digitalPinToInterrupt(pin), 
                            []() {
                                // Static ISR wrapper
                                if (instance && instance->interruptHandler) {
                                    instance->interruptHandler(instance->currentChannel);
                                }
                                instance->interruptFlag = true;
                            }, 
                            arduinoMode);
        }
    }

    void MUXManager::detachInterrupt() {
        if (interruptPin != 255) {
            ::detachInterrupt(digitalPinToInterrupt(interruptPin));
            interruptHandler = nullptr;
            interruptPin = 255;
            interruptFlag = false;
        }
    }

    bool MUXManager::startScan(uint8_t startChannel, uint8_t endChannel) {
        return false;
    }

    void MUXManager::stopScan() {
    }

    void MUXManager::sleep() {
    }

    void MUXManager::wake() {
    }

    bool MUXManager::selfTest() {
        return true;
    }

    uint16_t MUXManager::readDiagnostics() {
        return 0;
    }

    bool MUXManager::isValidChannel(uint8_t channel) const {
        return channel < maxChannels;
    }

    void MUXManager::delayMicros(unsigned int us) {
        #if defined(ESP8266) || defined(ESP32)
            ets_delay_us(us);
        #else
            delayMicroseconds(us);
        #endif
    }

    // Optional: Static helper functions that might be useful across different MUX types
    namespace Utility {
        uint8_t reverseBits(uint8_t b) {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
        }

        uint16_t calculateCRC(uint8_t* data, uint8_t length) {
            uint16_t crc = 0xFFFF;
            
            for (uint8_t i = 0; i < length; i++) {
                crc ^= data[i];
                for (uint8_t j = 0; j < 8; j++) {
                    if (crc & 0x0001) {
                        crc >>= 1;
                        crc ^= 0xA001;
                    } else {
                        crc >>= 1;
                    }
                }
            }
            
            return crc;
        }

        bool isChannelInRange(uint8_t channel, uint8_t maxChannels) {
            return channel < maxChannels;
        }

        uint8_t calculateRequiredSelectPins(uint8_t channels) {
            uint8_t pins = 0;
            while ((1 << pins) < channels) {
                pins++;
            }
            return pins;
        }
    }
}