// Specialized Multiplexer Module (SpecializedMUX.h)
#ifndef SPECIALIZED_MUX_H
#define SPECIALIZED_MUX_H

#include "MUXLib.h"

// Platform-specific fast GPIO handling
#if defined(ESP32)
    #define FAST_GPIO_AVAILABLE
    #include "driver/gpio.h"
#elif defined(ESP8266)
    #define FAST_GPIO_AVAILABLE
    #include "gpio.h"
#endif

namespace MUXLib {
    // Fast Digital Multiplexer base class with optimized GPIO handling
    class FastMUX : public MUXManager {
    protected:
        uint8_t* pins;
        uint8_t numPins;
        uint32_t pinMask;
        
        #ifdef FAST_GPIO_AVAILABLE
        void fastDigitalWrite(uint8_t pin, bool value) {
            #if defined(ESP32)
                if (value) {
                    GPIO.out_w1ts = (1 << pin);
                } else {
                    GPIO.out_w1tc = (1 << pin);
                }
            #elif defined(ESP8266)
                if (value) {
                    GPOS = (1 << pin);
                } else {
                    GPOC = (1 << pin);
                }
            #endif
        }
        #else
        void fastDigitalWrite(uint8_t pin, bool value) {
            digitalWrite(pin, value);
        }
        #endif
        
    public:
        FastMUX(uint8_t* controlPins, uint8_t pinCount, uint8_t maxChannels) 
            : MUXManager(0, maxChannels), numPins(pinCount), pinMask(0) {
            pins = (uint8_t*)malloc(pinCount * sizeof(uint8_t));
            if (pins) {
                memcpy(pins, controlPins, pinCount * sizeof(uint8_t));
            }
        }
        
        ~FastMUX() {
            if (pins) {
                free(pins);
            }
        }
        
        MUXStatus begin() override {
            if (!pins) return MUXStatus::ERROR_INIT;
            
            for (uint8_t i = 0; i < numPins; i++) {
                pinMode(pins[i], OUTPUT);
                digitalWrite(pins[i], LOW);
                pinMask |= (1 << pins[i]);
            }
            
            enable();
            return MUXStatus::OK;
        }
    };

    // Video Multiplexer for composite/component video
    class VideoMUX : public FastMUX {
    private:
        uint8_t syncPin;
        bool syncEnabled;
        uint8_t videoType; // 0=composite, 1=component
        
    public:
        VideoMUX(uint8_t* controlPins, uint8_t pinCount, uint8_t sync = 255, uint8_t type = 0)
            : FastMUX(controlPins, pinCount, 1 << pinCount),
              syncPin(sync), syncEnabled(false), videoType(type) {}
              
        MUXStatus begin() override {
            MUXStatus status = FastMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (syncPin != 255) {
                pinMode(syncPin, INPUT);
                syncEnabled = true;
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Wait for vertical sync if enabled
            if (syncEnabled) {
                while (digitalRead(syncPin) == HIGH) {
                    delayMicros(1);
                }
            }
            
            // Fast channel switching
            for (uint8_t i = 0; i < numPins; i++) {
                fastDigitalWrite(pins[i], (channel >> i) & 0x01);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        void setSyncEnabled(bool enable) {
            syncEnabled = enable && (syncPin != 255);
        }
    };

    // Audio Multiplexer with fade control
    class AudioMUX : public FastMUX {
    private:
        uint8_t fadeSteps;
        uint16_t fadeDelay;
        bool useFading;
        
    public:
        AudioMUX(uint8_t* controlPins, uint8_t pinCount)
            : FastMUX(controlPins, pinCount, 1 << pinCount),
              fadeSteps(16), fadeDelay(1), useFading(true) {}
              
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            if (useFading) {
                // Fade out
                for (uint8_t i = 0; i < fadeSteps; i++) {
                    delayMicros(fadeDelay);
                }
            }
            
            // Switch channel
            for (uint8_t i = 0; i < numPins; i++) {
                fastDigitalWrite(pins[i], (channel >> i) & 0x01);
            }
            
            if (useFading) {
                // Fade in
                for (uint8_t i = 0; i < fadeSteps; i++) {
                    delayMicros(fadeDelay);
                }
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        void configureFade(uint8_t steps, uint16_t delayUs, bool enable = true) {
            fadeSteps = steps;
            fadeDelay = delayUs;
            useFading = enable;
        }
    };

    // High-speed data multiplexer with buffer
    class DataMUX : public FastMUX {
    private:
        static const uint8_t BUFFER_SIZE = 32;
        uint8_t buffer[BUFFER_SIZE];
        uint8_t bufferIndex;
        bool buffering;
        
    public:
        DataMUX(uint8_t* controlPins, uint8_t pinCount)
            : FastMUX(controlPins, pinCount, 1 << pinCount),
              bufferIndex(0), buffering(false) {
            memset(buffer, 0, BUFFER_SIZE);
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            if (buffering) {
                // Store in buffer if there's space
                if (bufferIndex < BUFFER_SIZE) {
                    buffer[bufferIndex++] = channel;
                    return MUXStatus::OK;
                }
                return MUXStatus::ERROR_OVERFLOW;
            }
            
            // Direct channel switch
            for (uint8_t i = 0; i < numPins; i++) {
                fastDigitalWrite(pins[i], (channel >> i) & 0x01);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        void startBuffering() {
            buffering = true;
            bufferIndex = 0;
        }
        
        MUXStatus flushBuffer() {
            buffering = false;
            for (uint8_t i = 0; i < bufferIndex; i++) {
                MUXStatus status = setChannel(buffer[i]);
                if (status != MUXStatus::OK) return status;
                delayMicros(1); // Minimal delay between switches
            }
            bufferIndex = 0;
            return MUXStatus::OK;
        }
        
        void clearBuffer() {
            bufferIndex = 0;
            memset(buffer, 0, BUFFER_SIZE);
        }
    };

    // High-precision multiplexer with calibration
    class PrecisionMUX : public FastMUX {
    private:
        int16_t* calibrationOffsets;
        uint16_t* calibrationGains;
        bool calibrated;
        
    public:
        PrecisionMUX(uint8_t* controlPins, uint8_t pinCount)
            : FastMUX(controlPins, pinCount, 1 << pinCount),
              calibrated(false) {
            calibrationOffsets = (int16_t*)malloc(maxChannels * sizeof(int16_t));
            calibrationGains = (uint16_t*)malloc(maxChannels * sizeof(uint16_t));
            if (calibrationOffsets && calibrationGains) {
                memset(calibrationOffsets, 0, maxChannels * sizeof(int16_t));
                for (uint8_t i = 0; i < maxChannels; i++) {
                    calibrationGains[i] = 1024; // Unity gain (10-bit fixed point)
                }
            }
        }
        
        ~PrecisionMUX() {
            if (calibrationOffsets) free(calibrationOffsets);
            if (calibrationGains) free(calibrationGains);
        }
        
        MUXStatus begin() override {
            if (!calibrationOffsets || !calibrationGains) {
                return MUXStatus::ERROR_INIT;
            }
            return FastMUX::begin();
        }
        
        void setCalibration(uint8_t channel, int16_t offset, uint16_t gain) {
            if (isValidChannel(channel) && calibrationOffsets && calibrationGains) {
                calibrationOffsets[channel] = offset;
                calibrationGains[channel] = gain;
                calibrated = true;
            }
        }
        
        int16_t applyCalibration(uint8_t channel, int16_t value) {
            if (!calibrated || !isValidChannel(channel)) return value;
            
            int32_t calibrated = value;
            calibrated += calibrationOffsets[channel];
            calibrated *= calibrationGains[channel];
            calibrated >>= 10; // Fixed point adjustment
            
            return (int16_t)constrain(calibrated, -32768, 32767);
        }
    };
}

#endif