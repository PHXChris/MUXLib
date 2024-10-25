// Core MUX Library Header (MUXLib.h)
// (C) Chirs Formeister 2024. All Rights Reserved

#ifndef MUXLIB_H
#define MUXLIB_H

namespace MUXLib {
    enum class MUXStatus {
        OK,
        ERROR_INIT,
        ERROR_COMMUNICATION,
        ERROR_CHANNEL_INVALID,
        ERROR_NOT_ENABLED,
        ERROR_OVERFLOW
    };

    enum class InterruptMode {
        NONE,
        LOW_LEVEL,
        HIGH_LEVEL,
        FALLING_EDGE,
        RISING_EDGE,
        CHANGE
    };

    // Platform-independent interrupt handling
    typedef void (*InterruptCallback)(uint8_t);

    class MUXManager {
    protected:
        uint8_t deviceAddress;
        bool enabled;
        uint8_t currentChannel;
        uint8_t maxChannels;
        InterruptCallback interruptHandler;
        volatile bool interruptFlag;
        uint8_t interruptPin;
        
    public:
        MUXManager(uint8_t address, uint8_t channels) 
            : deviceAddress(address), enabled(false), currentChannel(0),
              maxChannels(channels), interruptHandler(nullptr), 
              interruptFlag(false), interruptPin(255) {}
              
        virtual ~MUXManager() {
            if (interruptPin != 255) {
                detachInterrupt();
            }
        }
        
        virtual MUXStatus begin() = 0;
        virtual MUXStatus setChannel(uint8_t channel) = 0;
        virtual uint8_t getChannel() { return currentChannel; }
        
        virtual bool isEnabled() { return enabled; }
        virtual void enable() { enabled = true; }
        virtual void disable() { enabled = false; }
        
        // Platform-independent interrupt handling
        virtual void attachInterrupt(InterruptCallback callback, uint8_t pin, InterruptMode mode = InterruptMode::CHANGE) {
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
                ::attachInterrupt(digitalPinToInterrupt(pin), 
                                []() {
                                    // Static ISR wrapper
                                    if (this->interruptHandler) {
                                        this->interruptHandler(this->currentChannel);
                                    }
                                    this->interruptFlag = true;
                                }, 
                                arduinoMode);
            }
        }

        virtual void detachInterrupt() {
            if (interruptPin != 255) {
                ::detachInterrupt(digitalPinToInterrupt(interruptPin));
                interruptHandler = nullptr;
                interruptPin = 255;
            }
        }
        
        // Channel scanning
        virtual bool startScan(uint8_t startChannel = 0, uint8_t endChannel = 0) { 
            return false; 
        }
        virtual void stopScan() {}
        
        // Basic power management
        virtual void sleep() {}
        virtual void wake() {}
        
        // Simple diagnostics
        virtual bool selfTest() { return true; }
        virtual uint16_t readDiagnostics() { return 0; }
        
    protected:
        // Utility function for bounds checking
        bool isValidChannel(uint8_t channel) const {
            return channel < maxChannels;
        }
        
        // Platform-independent delay microseconds
        void delayMicros(unsigned int us) {
            #if defined(ESP8266) || defined(ESP32)
                ets_delay_us(us);
            #else
                delayMicroseconds(us);
            #endif
        }
    };
}

#endif