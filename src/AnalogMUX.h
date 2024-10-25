// AnalogMUX.h
#ifndef ANALOGMUX_H
#define ANALOGMUX_H

#include "MUXLib.h"

namespace MUXLib {
    // Base class for analog multiplexers
    class AnalogMUX : public MUXManager {
    protected:
        uint8_t* selectPins;
        uint8_t numSelectPins;
        uint8_t enablePin;
        uint8_t signalPin;
        uint16_t settlingTime;  // microseconds
        
    public:
        AnalogMUX(uint8_t* selPins, uint8_t numPins, uint8_t sigPin, uint8_t enPin = 255) 
            : MUXManager(0, 1 << numPins), numSelectPins(numPins), 
              enablePin(enPin), signalPin(sigPin), settlingTime(10) {
            selectPins = (uint8_t*)malloc(numPins * sizeof(uint8_t));
            if (selectPins) {
                memcpy(selectPins, selPins, numPins * sizeof(uint8_t));
            }
        }
        
        ~AnalogMUX() {
            if (selectPins) {
                free(selectPins);
            }
        }
        
        MUXStatus begin() override {
            if (!selectPins) return MUXStatus::ERROR_INIT;
            
            for (uint8_t i = 0; i < numSelectPins; i++) {
                pinMode(selectPins[i], OUTPUT);
                digitalWrite(selectPins[i], LOW);
            }
            
            if (enablePin != 255) {
                pinMode(enablePin, OUTPUT);
                digitalWrite(enablePin, HIGH);  // Most analog muxes are active LOW
            }
            
            if (signalPin != 255) {
                pinMode(signalPin, INPUT);
            }
            
            enable();
            return MUXStatus::OK;
        }
        
        void setSettlingTime(uint16_t microseconds) {
            settlingTime = microseconds;
        }
        
        virtual uint16_t readChannel(uint8_t channel) {
            if (setChannel(channel) != MUXStatus::OK) {
                return 0;
            }
            
            delayMicros(settlingTime);
            return analogRead(signalPin);
        }
    };

    // 74HC4051 - 8 channel analog multiplexer
    class HC4051 : public AnalogMUX {
    public:
        HC4051(uint8_t* selPins, uint8_t sigPin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 3, sigPin, enPin) {}  // 3 select pins for 8 channels
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching (break-before-make)
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
    };

    // 74HC4067 - 16 channel analog multiplexer
    class HC4067 : public AnalogMUX {
    public:
        HC4067(uint8_t* selPins, uint8_t sigPin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 4, sigPin, enPin) {}  // 4 select pins for 16 channels
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching (break-before-make)
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < 4; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
    };

    // 74HC4052 - Dual 4-channel analog multiplexer
    class HC4052 : public AnalogMUX {
    private:
        uint8_t signalPin2;  // Second signal pin for the dual multiplexer
        
    public:
        HC4052(uint8_t* selPins, uint8_t sig1Pin, uint8_t sig2Pin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 2, sig1Pin, enPin), signalPin2(sig2Pin) {}  // 2 select pins for 4 channels each
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (signalPin2 != 255) {
                pinMode(signalPin2, INPUT);
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (channel >= 4) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < 2; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        // Read from the second multiplexer
        uint16_t readChannel2(uint8_t channel) {
            if (setChannel(channel) != MUXStatus::OK) {
                return 0;
            }
            
            delayMicros(settlingTime);
            return analogRead(signalPin2);
        }
    };

    // 74HC4053 - Triple 2-channel analog multiplexer
    class HC4053 : public AnalogMUX {
    private:
        uint8_t signalPin2;
        uint8_t signalPin3;
        
    public:
        HC4053(uint8_t* selPins, uint8_t sig1Pin, uint8_t sig2Pin, uint8_t sig3Pin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 3, sig1Pin, enPin), 
              signalPin2(sig2Pin), signalPin3(sig3Pin) {}  // 3 independent select pins
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (signalPin2 != 255) pinMode(signalPin2, INPUT);
            if (signalPin3 != 255) pinMode(signalPin3, INPUT);
            
            return MUXStatus::OK;
        }
        
        // Set individual switches (0 or 1 for each)
        MUXStatus setChannels(bool ch1, bool ch2, bool ch3) {
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            digitalWrite(selectPins[0], ch1);
            digitalWrite(selectPins[1], ch2);
            digitalWrite(selectPins[2], ch3);
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            return MUXStatus::OK;
        }
        
        uint16_t readChannel2() {
            delayMicros(settlingTime);
            return analogRead(signalPin2);
        }
        
        uint16_t readChannel3() {
            delayMicros(settlingTime);
            return analogRead(signalPin3);
        }
    };

    // ADG508A/ADG509A - 8-channel analog multiplexer
    class ADG508A : public AnalogMUX {
    private:
        bool isDifferential;  // ADG509A is differential version
        uint8_t signalPinB;  // For differential mode (ADG509A)
        
    public:
        ADG508A(uint8_t* selPins, uint8_t sigPin, uint8_t enPin = 255, 
                bool differential = false, uint8_t sigPinB = 255)
            : AnalogMUX(selPins, 3, sigPin, enPin), 
              isDifferential(differential), signalPinB(sigPinB) {}
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (isDifferential && signalPinB != 255) {
                pinMode(signalPinB, INPUT);
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        // For differential mode (ADG509A)
        int16_t readDifferential(uint8_t channel) {
            if (!isDifferential || signalPinB == 255) return 0;
            
            setChannel(channel);
            delayMicros(settlingTime);
            return analogRead(signalPin) - analogRead(signalPinB);
        }
    };

    // ADG706/ADG707 16:1 Multiplexer
    class ADG706 : public AnalogMUX {
    private:
        uint8_t writePin;
        bool isDifferential;  // ADG707 is differential version
        uint8_t signalPinB;
        
    public:
        ADG706(uint8_t* addrPins, uint8_t sigPin, uint8_t wrPin, 
               uint8_t enPin = 255, bool differential = false, uint8_t sigPinB = 255)
            : AnalogMUX(addrPins, 4, sigPin, enPin),
              writePin(wrPin), isDifferential(differential), signalPinB(sigPinB) {}
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            pinMode(writePin, OUTPUT);
            digitalWrite(writePin, HIGH);
            
            if (isDifferential && signalPinB != 255) {
                pinMode(signalPinB, INPUT);
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Set address before write pulse
            for (uint8_t i = 0; i < 4; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            // Generate write pulse
            digitalWrite(writePin, LOW);
            delayMicros(1);
            digitalWrite(writePin, HIGH);
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        int16_t readDifferential(uint8_t channel) {
            if (!isDifferential || signalPinB == 255) return 0;
            
            setChannel(channel);
            delayMicros(settlingTime);
            return analogRead(signalPin) - analogRead(signalPinB);
        }
    };

    // ADG506A/ADG507A - 16/8 channel multiplexer with differential capability
    class ADG506A : public AnalogMUX {
    private:
        bool isDifferential;
        uint8_t signalPinB;
        bool is506;  // true for ADG506A, false for ADG507A
        
    public:
        ADG506A(uint8_t* addrPins, uint8_t sigPin, bool is506A = true,
                uint8_t enPin = 255, bool differential = false, uint8_t sigPinB = 255)
            : AnalogMUX(addrPins, is506A ? 4 : 3, sigPin, enPin),
              isDifferential(differential), signalPinB(sigPinB), is506(is506A) {}
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            if (isDifferential && signalPinB != 255) {
                pinMode(signalPinB, INPUT);
            }
            
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            uint8_t maxChannel = is506 ? 16 : 8;
            if (channel >= maxChannel) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Disable before switching
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < numSelectPins; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            returncurrentChannel = channel;
            return MUXStatus::OK;
        }
        
        int16_t readDifferential(uint8_t channel) {
            if (!isDifferential || signalPinB == 255) return 0;
            
            setChannel(channel);
            delayMicros(settlingTime);
            return analogRead(signalPin) - analogRead(signalPinB);
        }
    };

    // MPC506A/MPC507A - Pin compatible with ADG506A/ADG507A
    class MPC506A : public ADG506A {
    public:
        MPC506A(uint8_t* addrPins, uint8_t sigPin, bool is506A = true,
                uint8_t enPin = 255, bool differential = false, uint8_t sigPinB = 255)
            : ADG506A(addrPins, sigPin, is506A, enPin, differential, sigPinB) {
            // MPC506A/507A are functionally identical to ADG506A/507A
            // but have different electrical characteristics
            setSettlingTime(20);  // Longer settling time for MPC series
        }
    };

    // DG408/DG409 - 8-channel differential multiplexer
    class DG408 : public AnalogMUX {
    private:
        uint8_t signalPinB;
        bool isDG409;  // DG409 has different channel addressing
        
    public:
        DG408(uint8_t* selPins, uint8_t sigPin, uint8_t sigPinB, 
              uint8_t enPin = 255, bool isDG409Mode = false)
            : AnalogMUX(selPins, 3, sigPin, enPin),
              signalPinB(sigPinB), isDG409(isDG409Mode) {
            setSettlingTime(150);  // Default settling time for DG408/409
        }
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            pinMode(signalPinB, INPUT);
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (channel >= 8) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // DG409 has different channel addressing pattern
            if (isDG409) {
                channel = (channel & 0x04) | ((channel & 0x02) >> 1) | 
                         ((channel & 0x01) << 1);
            }
            
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
        
        int16_t readDifferential(uint8_t channel) {
            setChannel(channel);
            delayMicros(settlingTime);
            return analogRead(signalPin) - analogRead(signalPinB);
        }
    };

    // MAX4051A - 8-channel analog multiplexer with low resistance
    class MAX4051A : public AnalogMUX {
    public:
        MAX4051A(uint8_t* selPins, uint8_t sigPin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 3, sigPin, enPin) {
            setSettlingTime(5);  // Faster settling time than HC4051
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            if (enablePin != 255) {
                digitalWrite(enablePin, HIGH);
                delayMicros(1);
            }
            
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            if (enablePin != 255) {
                delayMicros(1);
                digitalWrite(enablePin, LOW);
            }
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
    };

    // MAX4582 - Precision 8:1 analog multiplexer
    class MAX4582 : public AnalogMUX {
    private:
        uint8_t loadPin;  // Load pin for synchronous operation
        
    public:
        MAX4582(uint8_t* selPins, uint8_t sigPin, uint8_t ldPin, uint8_t enPin = 255)
            : AnalogMUX(selPins, 3, sigPin, enPin), loadPin(ldPin) {
            setSettlingTime(15);
        }
        
        MUXStatus begin() override {
            MUXStatus status = AnalogMUX::begin();
            if (status != MUXStatus::OK) return status;
            
            pinMode(loadPin, OUTPUT);
            digitalWrite(loadPin, HIGH);
            return MUXStatus::OK;
        }
        
        MUXStatus setChannel(uint8_t channel) override {
            if (!isValidChannel(channel)) return MUXStatus::ERROR_CHANNEL_INVALID;
            if (!enabled) return MUXStatus::ERROR_NOT_ENABLED;
            
            // Set up address bits while load is high
            for (uint8_t i = 0; i < 3; i++) {
                digitalWrite(selectPins[i], (channel >> i) & 0x01);
            }
            
            // Generate load pulse
            digitalWrite(loadPin, LOW);
            delayMicros(1);
            digitalWrite(loadPin, HIGH);
            
            currentChannel = channel;
            return MUXStatus::OK;
        }
    };
}

#endif