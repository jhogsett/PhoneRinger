#include "TelephoneRinger.h"

TelephoneRinger::TelephoneRinger() {
    state = IDLE;
    relayPin = -1;
    lastStateChange = 0;
    currentRingCount = 0;
    totalRingsToMake = 0;
    finalRingCutShort = false;
    waitDuration = 0;
}

void TelephoneRinger::initialize(int pin) {
    relayPin = pin;
    state = IDLE;
    lastStateChange = millis();
    // Start with a random delay before first call
    waitDuration = getRandomWaitTime();
}

void TelephoneRinger::step(unsigned long currentTime) {
    unsigned long elapsed = currentTime - lastStateChange;
    
    switch (state) {
        case IDLE:
            // Check if it's time to start a new call
            if (elapsed >= waitDuration) {
                startCall();
                lastStateChange = currentTime;
            }
            break;
            
        case RING_ON:
            // Check if ring duration is complete
            if ((finalRingCutShort && currentRingCount == totalRingsToMake && elapsed >= SHORT_RING_DURATION) ||
                (!finalRingCutShort && elapsed >= RING_ON_DURATION)) {
                
                setRelayState(false); // Turn off ring
                
                if (currentRingCount >= totalRingsToMake) {
                    // Call sequence complete
                    state = CALL_ANSWERED;
                } else {
                    // More rings to go
                    state = RING_OFF;
                }
                lastStateChange = currentTime;
            }
            break;
            
        case RING_OFF:
            // Check if silence duration is complete
            if (elapsed >= RING_OFF_DURATION) {
                currentRingCount++;
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
            }
            break;
            
        case WAITING:
            // Wait for the specified duration before going idle (ready for next call)
            if (elapsed >= waitDuration) {
                state = IDLE;
                waitDuration = getRandomWaitTime();
                lastStateChange = currentTime;
            }
            break;
    }
}

void TelephoneRinger::startCall() {
    totalRingsToMake = getRandomRingCount();
    currentRingCount = 1;
    finalRingCutShort = shouldCutShortFinalRing();
    
    setRelayState(true); // Turn on first ring
    state = RING_ON;
    
    // Debug output
    Serial.print("Phone on pin ");
    Serial.print(relayPin);
    Serial.print(" starting call: ");
    Serial.print(totalRingsToMake);
    Serial.print(" rings");
    if (finalRingCutShort) {
        Serial.print(" (final ring cut short)");
    }
    Serial.println();
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
        digitalWrite(relayPin, active ? HIGH : LOW);
    }
}

unsigned long TelephoneRinger::getRandomWaitTime() {
    return random(MIN_WAIT_TIME, MAX_WAIT_TIME + 1);
}

int TelephoneRinger::getRandomRingCount() {
    return random(1, 9); // 1 to 8 rings
}

bool TelephoneRinger::shouldCutShortFinalRing() {
    // If more than 1 ring, 70% chance the final ring gets cut short
    return (totalRingsToMake > 1) && (random(100) < 70);
}
