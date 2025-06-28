#include "TelephoneRinger.h"
#include "RingerManager.h"
#include "DisplayManager.h"

// Hardware pin definitions - Updated for your specific setup
const int RELAY_PINS[] = {5, 6, 7, 8, 9, 10, 11, 12}; // Digital pins 5-12 for 8-relay module
const int NUM_PHONES = 8;

// UI Hardware pins
const int ENCODER_PIN_A = 2;      // Encoder A
const int ENCODER_PIN_B = 3;      // Encoder B  
const int ENCODER_BUTTON = 4;     // Encoder button
const int PAUSE_BUTTON = A0;      // System pause button (moved from pin 13)
// I2C pins A4 (SDA) and A5 (SCL) for 20x4 LCD display

// System state
bool systemPaused = false;
bool lastPauseButtonState = HIGH;  // Assuming pull-up resistor
bool pauseButtonPressed = false;   // Track if button was just pressed
unsigned long lastPauseDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// Create the system components
RingerManager ringerManager;
DisplayManager displayManager;

// Function declarations
void checkPauseButton();

void setup() {
  Serial.begin(9600);
  Serial.println(F("Call Center Simulator Starting..."));
  Serial.println(F("Hardware: 8-Relay Module + 20x4 LCD + Rotary Encoder + Pause Button"));
  
  // Seed the random number generator with analog noise
  randomSeed(analogRead(A0));
  
  // Initialize relay pins as outputs (active LOW for most relay modules)
  for (int i = 0; i < NUM_PHONES; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH); // Start with relays off (HIGH = inactive for active-LOW modules)
  }
  
  // Initialize control pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);
  pinMode(PAUSE_BUTTON, INPUT_PULLUP);
  
  // Initialize the ringer manager with phone instances (using nullptr for config for now)
  ringerManager.initialize(RELAY_PINS, NUM_PHONES, nullptr);
  
  // Initialize the display
  displayManager.initialize();
  
  Serial.println(F("Call Center Simulator Ready!"));
  Serial.println(F("Hardware Configuration:"));
  Serial.println(F("- 8 phone lines on pins 5-12"));
  Serial.print(F("- Actual pin assignments: "));
  for (int i = 0; i < NUM_PHONES; i++) {
    Serial.print(RELAY_PINS[i]);
    if (i < NUM_PHONES - 1) Serial.print(F(", "));
  }
  Serial.println();
  Serial.println(F("- Rotary encoder on pins 2,3,4"));
  Serial.println(F("- Pause button on pin A0"));
  Serial.println(F("- 20x4 LCD on I2C (A4/A5)"));
  Serial.println(F("Press pause button (pin A0) to stop/start all activity"));
  
  // Test each relay briefly to verify connections
  Serial.println(F("Testing relay connections..."));
  for (int i = 0; i < NUM_PHONES; i++) {
    Serial.print(F("Testing relay "));
    Serial.print(i + 1);
    Serial.print(F(" on pin "));
    Serial.println(RELAY_PINS[i]);
    digitalWrite(RELAY_PINS[i], LOW);  // LOW = active for active-LOW modules
    delay(200);
    digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW modules
    delay(100);
  }
  Serial.println(F("Relay test complete. Starting normal operation..."));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check pause button
  checkPauseButton();
  
  // Only step the ringer manager if not paused
  if (!systemPaused) {
    ringerManager.step(currentTime);
  }
  
  // Update display
  displayManager.update(currentTime, systemPaused, &ringerManager);
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void checkPauseButton() {
  bool currentButtonState = digitalRead(PAUSE_BUTTON);
  
  // Debug output every 1000ms to see button state
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000) {
    Serial.print(F("Button: "));
    Serial.print(currentButtonState ? "HIGH" : "LOW");
    Serial.print(F(" | Paused: "));
    Serial.println(systemPaused ? "YES" : "NO");
    lastDebugTime = millis();
  }
  
  // Simple debounce: if button state changed, reset debounce timer
  if (currentButtonState != lastPauseButtonState) {
    lastPauseDebounce = millis();
    pauseButtonPressed = false; // Reset press flag
    Serial.print(F("Button changed: "));
    Serial.print(lastPauseButtonState ? "HIGH" : "LOW");
    Serial.print(F(" -> "));
    Serial.println(currentButtonState ? "HIGH" : "LOW");
  }
  
  // Check if enough time has passed for debounce
  if ((millis() - lastPauseDebounce) > DEBOUNCE_DELAY) {
    // If button is pressed (LOW) and we haven't processed this press yet
    if (currentButtonState == LOW && !pauseButtonPressed) {
      pauseButtonPressed = true; // Mark as processed
      Serial.println(F("*** PAUSE BUTTON PRESSED - TOGGLING STATE ***"));
      
      // Toggle pause state
      systemPaused = !systemPaused;
      
      if (systemPaused) {
        Serial.println(F("*** SYSTEM PAUSED - Stopping all activity ***"));
        // Stop all calls and turn off all relays immediately
        ringerManager.stopAllCalls();
        for (int i = 0; i < NUM_PHONES; i++) {
          digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW relay modules
        }
        displayManager.showPauseMessage();
      } else {
        Serial.println(F("*** SYSTEM RESUMED - Activity will restart ***"));
        displayManager.showResumeMessage();
      }
    }
    
    // Reset press flag when button is released
    if (currentButtonState == HIGH) {
      pauseButtonPressed = false;
    }
  }
  
  // Update the last button state
  lastPauseButtonState = currentButtonState;
}
