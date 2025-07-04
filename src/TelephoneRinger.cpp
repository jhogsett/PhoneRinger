#include "TelephoneRinger.h"
// #include "Config.h"  // Commented out for now to avoid dependencies

TelephoneRinger::TelephoneRinger() {
    state = IDLE;
    relayPin = -1;
    lastStateChange = 0;
    currentRingCount = 0;
    totalRingsToMake = 0;
    finalRingCutShort = false;
    useUKRingStyle = false;
    waitDuration = 0;
    enableSerialOutput = true;  // Default to enabled
    systemConfig = nullptr;
    canStartCallCallback = nullptr;
    currentRingOnDuration = 2000;   // Default 2 seconds
    currentRingOffDuration = 4000;  // Default 4 seconds
}

void TelephoneRinger::initialize(int pin, const SystemConfig* config, bool enableSerialOutput) {
    relayPin = pin;
    systemConfig = config;
    this->enableSerialOutput = enableSerialOutput;  // Store the flag
    state = IDLE;
    lastStateChange = millis();
    useUKRingStyle = false;
    // Set default timing values
    currentRingOnDuration = 2000;   // 2 seconds
    currentRingOffDuration = 4000;  // 4 seconds
    // Start with a random delay before first call
    waitDuration = getRandomWaitTime();
    if (enableSerialOutput) {
        Serial.print("Phone initialized on pin ");
        Serial.println(relayPin);
    }
}

void TelephoneRinger::setCanStartCallCallback(CanStartCallCallback callback) {
    canStartCallCallback = callback;
}

void TelephoneRinger::step(unsigned long currentTime) {
    unsigned long elapsed = currentTime - lastStateChange;
    
    switch (state) {
        case IDLE:
            // Check if it's time to start a new call AND if we're allowed to start one
            if (elapsed >= waitDuration) {
                // Check if callback allows starting a new call
                if (canStartCallCallback == nullptr || canStartCallCallback()) {
                    startCall();
                } else {
                    // Can't start a call now due to concurrent limit, wait a bit longer
                    waitDuration = getRandomWaitTime() / 4;  // Shorter wait before trying again
                    lastStateChange = currentTime;
                }
            }
            break;
            
        case RING_ON:
            // Check if ring duration is complete
            {
                unsigned long ringDuration = currentRingOnDuration;
                
                // If this is the final ring and it should be cut short, reduce the duration
                if (currentRingCount == totalRingsToMake && finalRingCutShort) {
                    // Cut the ring short by 25-75% (random)
                    ringDuration = currentRingOnDuration * random(25, 76) / 100;
                    if (enableSerialOutput) {
                        Serial.print("Phone pin ");
                        Serial.print(relayPin);
                        Serial.print(" final ring cut short to ");
                        Serial.print(ringDuration);
                        Serial.println("ms");
                    }
                }
                
                if (elapsed >= ringDuration) {
                    setRelayState(false); // Turn off ring
                    if (enableSerialOutput) {
                        Serial.print("Phone pin ");
                        Serial.print(relayPin);
                        Serial.print(" ring ");
                        Serial.print(currentRingCount);
                        Serial.print("/");
                        Serial.print(totalRingsToMake);
                        Serial.println(" OFF");
                    }
                    
                    if (currentRingCount >= totalRingsToMake) {
                        // Call sequence complete
                        state = CALL_ANSWERED;
                        if (enableSerialOutput) {
                            Serial.print("Phone pin ");
                            Serial.print(relayPin);
                            Serial.println(" call complete");
                        }
                    } else {
                        // More rings to go
                        state = RING_OFF;
                    }
                    lastStateChange = currentTime;
                }
            }
            break;
            
        case RING_OFF:
            // Check if silence duration is complete
            if (elapsed >= currentRingOffDuration) {
                currentRingCount++;
                if (enableSerialOutput) {
                    Serial.print("Phone pin ");
                    Serial.print(relayPin);
                    Serial.print(" starting ring ");
                    Serial.print(currentRingCount);
                    Serial.print("/");
                    Serial.println(totalRingsToMake);
                }
                setRelayState(true); // Turn on next ring
                state = RING_ON;
                lastStateChange = currentTime;
            }
            break;
            
        case CALL_ANSWERED:
            // Brief pause after call ends, then wait for next call
            if (elapsed >= 1000) { // 1 second pause
                state = WAITING;
                waitDuration = getRandomWaitTime();
                lastStateChange = currentTime;
                if (enableSerialOutput) {
                    Serial.print("Phone pin ");
                    Serial.print(relayPin);
                    Serial.print(" waiting ");
                    Serial.print(waitDuration);
                    Serial.println("ms for next call");
                }
            }
            break;
            
        case WAITING:
            // Wait for the specified duration before going idle (ready for next call)
            if (elapsed >= waitDuration) {
                state = IDLE;
                waitDuration = getRandomWaitTime();
                lastStateChange = currentTime;
                if (enableSerialOutput) {
                    Serial.print("Phone pin ");
                    Serial.print(relayPin);
                    Serial.println(" ready for next call");
                }
            }
            break;
    }
}

void TelephoneRinger::startCall() {
    // Original simple logic: 1-8 random rings, last ring sometimes cut short
    totalRingsToMake = random(1, 9); // 1 to 8 rings
    currentRingCount = 1;
    
    // 50% chance that the final ring gets cut short (to simulate someone answering)
    finalRingCutShort = (random(100) < 50);
    
    if (enableSerialOutput) {
        Serial.print("Phone on pin ");
        Serial.print(relayPin);
        Serial.print(" starting call: ");
        Serial.print(totalRingsToMake);
        Serial.print(" rings");
        if (finalRingCutShort) {
            Serial.print(" (last ring may be cut short)");
        }
        Serial.println();
    }
    
    setRelayState(true); // Turn on first ring
    state = RING_ON;
    lastStateChange = millis(); // Reset timer for the RING_ON state
}

void TelephoneRinger::startCall(int ringCount, bool cutShort, bool useUKStyleRing) {
    totalRingsToMake = ringCount;
    currentRingCount = 1;
    finalRingCutShort = cutShort;
    useUKRingStyle = useUKStyleRing;
    
    // Set default ring timing (simplified)
    currentRingOnDuration = 2000;   // 2 seconds
    currentRingOffDuration = 4000;  // 4 seconds
    
    setRelayState(true); // Turn on first ring
    state = RING_ON;
    lastStateChange = millis(); // Reset timer for the RING_ON state
    
    // Debug output using safe char buffer instead of String concatenation
    if (enableSerialOutput) {
        Serial.print(F("Phone on pin "));
        Serial.print(relayPin);
        Serial.print(F(" starting call: "));
        Serial.print(totalRingsToMake);
        Serial.print(F(" rings"));
        if (finalRingCutShort) {
            Serial.print(F(" (final ring cut short)"));
        }
        Serial.println();
    }
}

void TelephoneRinger::stopCall() {
    setRelayState(false);
    state = IDLE;
    waitDuration = getRandomWaitTime();
    lastStateChange = millis();
}

bool TelephoneRinger::isRinging() const {
    return state == RING_ON;
}

bool TelephoneRinger::isActive() const {
    return state != IDLE && state != WAITING;
}

void TelephoneRinger::setRelayState(bool active) {
    if (relayPin >= 0) {
        // Most relay modules are active LOW, so invert the logic
        digitalWrite(relayPin, active ? LOW : HIGH);
        if (enableSerialOutput) {
            Serial.print("Relay pin ");
            Serial.print(relayPin);
            Serial.print(" set to ");
            Serial.println(active ? "ON (LOW)" : "OFF (HIGH)");
        }
    }
}

unsigned long TelephoneRinger::getRandomWaitTime() {
    // Use global maxCallDelaySetting from main.cpp
    // Convert seconds to milliseconds and create random range from 5s to maxCallDelaySetting
    unsigned long minDelay = 5000;  // 5 seconds minimum
    unsigned long maxDelay = (unsigned long)maxCallDelaySetting * 1000;  // Convert to milliseconds
    
    // Ensure minimum is not greater than maximum
    if (minDelay >= maxDelay) {
        minDelay = maxDelay / 2;  // Set minimum to half of maximum if needed
    }
    
    return random(minDelay, maxDelay + 1);
}
