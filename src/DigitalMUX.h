// Digital Multiplexer Module (DigitalMUX.h)
#ifndef DIGITALMUX_H
#define DIGITALMUX_H

#include "MUXLib.h"

// Platform-specific SPI handling
#if defined(ESP8266) || defined(ESP32)
    #include <SPI.h>
    #define SPI_AVAILABLE
#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
    #include <SPI.h>
    #define SPI_AVAILABLE
#endif

namespace MUXLib {
    // Base class for digital multiplexers with parallel interface
    class ParallelMUX : public MUXManager {
    protected:
        uint8_t selectPins[5];  // Support up to 5 select pins (32 channels)
        uint8_t numSelectPins;
        uint8_t enablePin;
        
    public:
        ParallelMUX(uint8_t* selPins, uint8_t numPins, uint8_t enPin = 255, uint8_t maxCh = 0) 
            : MUXManager(0, maxCh ? maxCh : (1 << numPins)), 
              numSelectPins(numPins), enablePin(enPin) {
            for (uint8_t i = 0; i < numPins && i < 5; i++) {
                selectPins[i] = selPins[i];
            }
        }
        
        MUXStatus begin() override {
            for (uint8_t i = 0; i < numSelectPins; i++) {
                if (selectPins[i] != 255) {
                    pinMode(selectPins[i], OUTPUT);
                    digitalWrite(selectPins[i], LOW);
                }
            }
            
            if (enablePin != 255) {
                pinMode(enablePin, OUTPUT);
                digitalWrite(enablePin, HIGH);
            }
            
            enable();
            return MUXStatus::OK;
        }
        
        void enable() override {
            if (enablePin != 255) {
                digitalWrite(enablePin, LOW);
            }
            enabled = true;
        }
        
        void disable() override {
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
            }
            enabled = false;
        }
    };

    // 74HC405X (4051/4052/4053) Multiplexer
    class HC405X : public ParallelMUX {
    private:
        uint8_t muxType; // 1=4051(8ch), 2=4052(4ch dual), 3=4053(2ch triple)
        
    public:
        HC405X(uint8_t* selPins, uint8_t type, uint8_t enPin = 255)
            : ParallelMUX(selPins, type == 1 ? 3 : (type == 2 ? 2 : 1), enPin,
                         type == 1 ? 8 : (type == 2 ? 4 : 2)),
              muxType(type) {}
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            for (uint8_t i = 0; i < numSelectPins; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
    };

    // CD74HC4067 16-Channel Multiplexer
    class CD74HC4067 : public ParallelMUX {
    private:
        uint8_t sigPin;
        bool autoRead;
        uint16_t* channelValues;
        
    public:
        CD74HC4067(uint8_t* selPins, uint8_t enPin = 255, uint8_t signalPin = 255)
            : ParallelMUX(selPins, 4, enPin, 16), sigPin(signalPin), 
              autoRead(false), channelValues(nullptr) {
            if (sigPin != 255) {
                channelValues = (uint16_t*)malloc(16 * sizeof(uint16_t));
                if (channelValues) {
                    memset(channelValues, 0, 16 * sizeof(uint16_t));
                }
            }
        }
        
        ~CD74HC4067() {
            if (channelValues) {
                free(channelValues);
            }
        }
        
        MUXStatus begin() override {
            MUXStatus status = ParallelMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (sigPin != 255) {
                pinMode(sigPin, INPUT);
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            for (uint8_t i = 0; i < 4; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (autoRead && sigPin != 255 && channelValues) {
                delayMicros(50); // Allow signal to settle
                channelValues[channel] = analogRead(sigPin);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        void enableAutoRead(bool enable = true) {
            if (sigPin != 255 && channelValues) {
                autoRead = enable;
            }
        }
        
        uint16_t readChannel(uint8_t channel) {
            if (sigPin == 255 || !isValidChannel(channel)) return 0;
            
            setChannel(channel);
            delayMicros(50); // Allow signal to settle
            return analogRead(sigPin);
        }
        
        uint16_t getChannelValue(uint8_t channel) {
            if (!channelValues || !isValidChannel(channel)) return 0;
            return channelValues[channel];
        }
    };

    // SPI-based digital multiplexer base class
    class SPIMUXBase : public MUXManager {
    protected:
        uint8_t csPin;
        uint8_t speedMHz;
        bool useHardwareSPI;
        
        #ifdef SPI_AVAILABLE
            SPISettings spiSettings;
        #endif
        
        // Software SPI pins (used when hardware SPI is not available)
        uint8_t mosiPin;
        uint8_t sckPin;
        
        void initSPI(uint8_t speed) {
            speedMHz = speed;
            #ifdef SPI_AVAILABLE
                if (useHardwareSPI) {
                    spiSettings = SPISettings(speed * 1000000UL, MSBFIRST, SPI_MODE0);
                    SPI.begin();
                }
            #endif
        }
        
        void spiTransfer(uint8_t data) {
            #ifdef SPI_AVAILABLE
                if (useHardwareSPI) {
                    SPI.beginTransaction(spiSettings);
                    digitalWrite(csPin, LOW);
                    SPI.transfer(data);
                    digitalWrite(csPin, HIGH);
                    SPI.endTransaction();
                } else
            #endif
            {
                // Software SPI implementation
                digitalWrite(csPin, LOW);
                for (int8_t i = 7; i >= 0; i--) {
                    digitalWrite(mosiPin, (data >> i) & 0x01);
                    digitalWrite(sckPin, HIGH);
                    delayMicros(1);
                    digitalWrite(sckPin, LOW);
                    delayMicros(1);
                }
                digitalWrite(csPin, HIGH);
            }
        }
        
    public:
        SPIMUXBase(uint8_t cs, uint8_t maxChannels, bool hwSPI = true,
                   uint8_t mosi = 255, uint8_t sck = 255)
            : MUXManager(0, maxChannels), csPin(cs), speedMHz(10),
              useHardwareSPI(hwSPI), mosiPin(mosi), sckPin(sck) {}
              
        MUXStatus begin() override {
            pinMode(csPin, OUTPUT);
            digitalWrite(csPin, HIGH);
            
            if (!useHardwareSPI) {
                pinMode(mosiPin, OUTPUT);
                pinMode(sckPin, OUTPUT);
                digitalWrite(mosiPin, LOW);
                digitalWrite(sckPin, LOW);
            }
            
            initSPI(speedMHz);
            enable();
            return MUXStatus::OK;
        }
        
        virtual void setSPISpeed(uint8_t speedMHz) {
            this->speedMHz = speedMHz;
            initSPI(speedMHz);
        }
    };
}

#endif