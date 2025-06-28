#include "TelephoneRinger.h"
#include "RingerManager.h"

// Define pin assignments for the 8 relays
const int RELAY_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9}; // Digital pins 2-9 for relays
const int NUM_PHONES = 8;

// Create the ringer manager
RingerManager ringerManager;

void setup() {
  Serial.begin(9600);
  Serial.println("Call Center Simulator Starting...");
  
  // Initialize all relay pins as outputs
  for (int i = 0; i < NUM_PHONES; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW); // Start with relays off (phones not ringing)
  }
  
  // Initialize the ringer manager with phone instances
  ringerManager.initialize(RELAY_PINS, NUM_PHONES);
  
  Serial.println("Call Center Simulator Ready!");
  Serial.println("8 phone lines active, incoming calls will begin...");
}

void loop() {
  // Step the ringer manager with current time
  ringerManager.step(millis());
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
