#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include <Arduino.h>

class EncoderManager {
public:
    // Encoder events
    enum EncoderEvent {
        NONE,
        CLOCKWISE,
        COUNTER_CLOCKWISE,
        BUTTON_PRESS,
        BUTTON_RELEASE,
        BUTTON_LONG_PRESS
    };
    
    EncoderManager();
    
    // Initialize encoder pins
    void initialize(int pinA, int pinB, int buttonPin, bool enableInitOutput = true);
    
    // Call this frequently in main loop to check for events
    EncoderEvent update();
    
    // Get current encoder state for debugging
    bool getButtonState() const;
    const char* getEventString(EncoderEvent event) const;
    
private:
    // Pin assignments
    int encoderPinA;
    int encoderPinB;
    int encoderButtonPin;
    
    // Encoder state tracking
    bool lastA;
    bool lastB;
    bool currentA;
    bool currentB;
    
    // Button state tracking
    bool lastButtonState;
    bool currentButtonState;
    bool lastRawButtonState;  // Track last raw state for debouncing
    bool buttonPressed;
    unsigned long buttonPressTime;
    unsigned long lastButtonDebounce;
    
    // Debug support
    unsigned long lastDebugPrint;
    static const unsigned long DEBUG_INTERVAL = 5000; // Print debug every 5 seconds
    
    // Timing constants
    static const unsigned long BUTTON_DEBOUNCE_TIME = 50;    // 50ms debounce
    static const unsigned long LONG_PRESS_TIME = 1000;       // 1 second for long press
    
    // Helper methods
    bool readEncoderA();
    bool readEncoderB();
    bool readButton();
    EncoderEvent checkRotation();
    EncoderEvent checkButton();
};

#endif
