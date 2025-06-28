#include "EncoderManager.h"

EncoderManager::EncoderManager() {
    encoderPinA = -1;
    encoderPinB = -1;
    encoderButtonPin = -1;
    lastA = false;
    lastB = false;
    currentA = false;
    currentB = false;
    lastButtonState = HIGH;
    currentButtonState = HIGH;
    lastRawButtonState = HIGH;
    buttonPressed = false;
    buttonPressTime = 0;
    lastButtonDebounce = 0;
    lastDebugPrint = 0;
}

void EncoderManager::initialize(int pinA, int pinB, int buttonPin, bool enableInitOutput) {
    encoderPinA = pinA;
    encoderPinB = pinB;
    encoderButtonPin = buttonPin;
    
    // Initialize pins (should already be done in main, but safe to repeat)
    pinMode(encoderPinA, INPUT_PULLUP);
    pinMode(encoderPinB, INPUT_PULLUP);
    pinMode(encoderButtonPin, INPUT_PULLUP);
    
    // Read initial states
    lastA = readEncoderA();
    lastB = readEncoderB();
    lastButtonState = readButton();
    lastRawButtonState = lastButtonState;
    currentButtonState = lastButtonState;
    
    if (enableInitOutput) {
        Serial.println(F("EncoderManager initialized"));
        Serial.print(F("Pin A: "));
        Serial.print(encoderPinA);
        Serial.print(F(", Pin B: "));
        Serial.print(encoderPinB);
        Serial.print(F(", Button: "));
        Serial.println(encoderButtonPin);
    }
}

EncoderManager::EncoderEvent EncoderManager::update() {
    // Check for rotation first
    EncoderEvent rotationEvent = checkRotation();
    if (rotationEvent != NONE) {
        return rotationEvent;
    }
    
    // Then check for button events
    return checkButton();
}

EncoderManager::EncoderEvent EncoderManager::checkRotation() {
    currentA = readEncoderA();
    currentB = readEncoderB();
    
    // Check if A pin changed
    if (currentA != lastA) {
        // A changed, check direction based on B
        if (currentA == currentB) {
            // Clockwise rotation
            lastA = currentA;
            lastB = currentB;
            Serial.println(F("Encoder: CLOCKWISE"));
            return CLOCKWISE;
        } else {
            // Counter-clockwise rotation
            lastA = currentA;
            lastB = currentB;
            Serial.println(F("Encoder: COUNTER_CLOCKWISE"));
            return COUNTER_CLOCKWISE;
        }
    }
    
    // Update last states
    lastA = currentA;
    lastB = currentB;
    
    return NONE;
}

EncoderManager::EncoderEvent EncoderManager::checkButton() {
    bool rawButtonState = readButton();
    unsigned long currentTime = millis();
    
    // Debug: Print button state changes immediately (disabled for clean operation)
    /*
    static bool lastRawState = HIGH;
    if (rawButtonState != lastRawState) {
        Serial.println(F("=== ENCODER BUTTON EVENT ==="));
        Serial.print(F("RAW BUTTON CHANGE: "));
        Serial.print(lastRawState ? "HIGH" : "LOW");
        Serial.print(F(" -> "));
        Serial.println(rawButtonState ? "HIGH" : "LOW");
        lastRawState = rawButtonState;
    }
    */
    
    // Debug: Print button state periodically (disabled for focused debugging)
    /*
    if (currentTime - lastDebugPrint > DEBUG_INTERVAL) {
        Serial.print(F("Button Debug - Raw: "));
        Serial.print(rawButtonState ? "HIGH" : "LOW");
        Serial.print(F(", Current: "));
        Serial.print(currentButtonState ? "HIGH" : "LOW");
        Serial.print(F(", Last: "));
        Serial.println(lastButtonState ? "HIGH" : "LOW");
        lastDebugPrint = currentTime;
    }
    */
    
    // Handle debouncing - only reset timer when raw state actually changes
    if (rawButtonState != lastRawButtonState) {
        lastButtonDebounce = currentTime; // Reset debounce timer on actual raw state change
        lastRawButtonState = rawButtonState; // Update the raw state tracker
        Serial.print(F("Button debounce reset, raw="));
        Serial.println(rawButtonState ? "HIGH" : "LOW");
    }
    
    // Check if button state has been stable for debounce time
    if ((currentTime - lastButtonDebounce) > BUTTON_DEBOUNCE_TIME) {
        // Update current state to the stable raw state
        bool newState = rawButtonState;
        
        // Check for state transitions
        if (newState != lastButtonState) {
            Serial.println(F("=== BUTTON STATE TRANSITION ==="));
            Serial.print(F("Button state change detected: "));
            Serial.print(lastButtonState ? "HIGH" : "LOW");
            Serial.print(F(" -> "));
            Serial.println(newState ? "HIGH" : "LOW");
            
            currentButtonState = newState;
            
            if (newState == LOW && lastButtonState == HIGH) {
                // Button just pressed
                buttonPressed = true;
                buttonPressTime = currentTime;
                Serial.println(F("*** ENCODER BUTTON PRESSED ***"));
                lastButtonState = newState;
                return BUTTON_PRESS;
            }
            
            if (newState == HIGH && lastButtonState == LOW) {
                // Button just released
                unsigned long pressDuration = currentTime - buttonPressTime;
                buttonPressed = false;
                Serial.print(F("*** ENCODER BUTTON RELEASED after "));
                Serial.print(pressDuration);
                Serial.println(F("ms ***"));
                lastButtonState = newState;
                return BUTTON_RELEASE;
            }
            
            Serial.println(F("=== DEBUG CONDITION CHECK ==="));
            Serial.print(F("newState="));
            Serial.print(newState);
            Serial.print(F(", lastButtonState="));
            Serial.print(lastButtonState);
            Serial.print(F(", LOW="));
            Serial.print(LOW);
            Serial.print(F(", HIGH="));
            Serial.println(HIGH);
            Serial.println(F("=== END DEBUG ==="));
            
            lastButtonState = newState;
        } else {
            // State is stable, just update current state
            currentButtonState = rawButtonState;
        }
    }
    
    // Check for long press while button is held (only if button is currently pressed)
    if (currentButtonState == LOW && buttonPressed) {
        unsigned long pressDuration = currentTime - buttonPressTime;
        if (pressDuration >= LONG_PRESS_TIME) {
            Serial.println(F("Encoder Button: LONG_PRESS"));
            buttonPressed = false; // Prevent repeated long press events
            return BUTTON_LONG_PRESS;
        }
    }
    
    return NONE;
}

bool EncoderManager::readEncoderA() {
    return digitalRead(encoderPinA);
}

bool EncoderManager::readEncoderB() {
    return digitalRead(encoderPinB);
}

bool EncoderManager::readButton() {
    return digitalRead(encoderButtonPin);
}

bool EncoderManager::getButtonState() const {
    return currentButtonState;
}

String EncoderManager::getEventString(EncoderEvent event) const {
    switch (event) {
        case NONE: return "NONE";
        case CLOCKWISE: return "CLOCKWISE";
        case COUNTER_CLOCKWISE: return "COUNTER_CLOCKWISE";
        case BUTTON_PRESS: return "BUTTON_PRESS";
        case BUTTON_RELEASE: return "BUTTON_RELEASE";
        case BUTTON_LONG_PRESS: return "BUTTON_LONG_PRESS";
        default: return "UNKNOWN";
    }
}
