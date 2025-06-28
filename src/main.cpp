#include "TelephoneRinger.h"
#include "RingerManager.h"
// #include "DisplayManager.h"  // Commented out to avoid LCD hanging issues

// Hardware pin definitions - Updated for your specific setup
const int RELAY_PINS[] = {5, 6, 7, 8, 9, 10, 11, 12}; // Digital pins 5-12 for 8-relay module
const int NUM_PHONES = 8;

// UI Hardware pins
const int ENCODER_PIN_A = 2;      // Encoder A
const int ENCODER_PIN_B = 3;      // Encoder B  
const int ENCODER_BUTTON = 4;     // Encoder button
const int PAUSE_BUTTON = 13;      // System pause button
// I2C pins A4 (SDA) and A5 (SCL) for 20x4 LCD display

// System state
bool systemPaused = false;
bool lastPauseButtonState = HIGH;  // Assuming pull-up resistor
unsigned long lastPauseDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// Create the system components
RingerManager ringerManager;
// DisplayManager displayManager; // Commented out to avoid LCD hanging issues

// Function declarations
void checkPauseButton();

void setup() {
  Serial.begin(9600);
  Serial.println("Call Center Simulator Starting...");
  Serial.println("Hardware: 8-Relay Module + 20x4 LCD + Rotary Encoder + Pause Button");
  
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
  // displayManager.initialize(); // Commented out to avoid LCD hanging issues
  
  Serial.println("Call Center Simulator Ready!");
  Serial.println("Hardware Configuration:");
  Serial.println("- 8 phone lines on pins 5-12");
  Serial.print("- Actual pin assignments: ");
  for (int i = 0; i < NUM_PHONES; i++) {
    Serial.print(RELAY_PINS[i]);
    if (i < NUM_PHONES - 1) Serial.print(", ");
  }
  Serial.println();
  Serial.println("- Rotary encoder on pins 2,3,4");
  Serial.println("- Pause button on pin 13");
  Serial.println("- 20x4 LCD on I2C (A4/A5)");
  Serial.println("Press pause button (pin 13) to stop/start all activity");
  
  // Test each relay briefly to verify connections
  Serial.println("Testing relay connections...");
  for (int i = 0; i < NUM_PHONES; i++) {
    Serial.print("Testing relay ");
    Serial.print(i + 1);
    Serial.print(" on pin ");
    Serial.println(RELAY_PINS[i]);
    digitalWrite(RELAY_PINS[i], LOW);  // LOW = active for active-LOW modules
    delay(200);
    digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW modules
    delay(100);
  }
  Serial.println("Relay test complete. Starting normal operation...");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check pause button
  checkPauseButton();
  
  // Only step the ringer manager if not paused
  if (!systemPaused) {
    ringerManager.step(currentTime);
  }
  
  // Update display (commented out since DisplayManager is not active)
  // displayManager.update(currentTime, systemPaused, &ringerManager);
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void checkPauseButton() {
  bool currentButtonState = digitalRead(PAUSE_BUTTON);
  
  // Check if button state changed
  if (currentButtonState != lastPauseButtonState) {
    lastPauseDebounce = millis();
  }
  
  // If button has been stable for debounce time
  if ((millis() - lastPauseDebounce) > DEBOUNCE_DELAY) {
    // If button was just pressed (LOW, due to pull-up)
    if (currentButtonState == LOW && lastPauseButtonState == HIGH) {
      // Toggle pause state
      systemPaused = !systemPaused;
      
      if (systemPaused) {
        // Stop all calls and turn off all relays immediately
        ringerManager.stopAllCalls();
        for (int i = 0; i < NUM_PHONES; i++) {
          digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW relay modules
        }
        Serial.println("*** SYSTEM PAUSED - All relays OFF ***");
        // displayManager.showPauseMessage(); // Commented out since DisplayManager is not active
      } else {
        Serial.println("*** SYSTEM RESUMED - Calls will restart ***");
        // displayManager.showResumeMessage(); // Commented out since DisplayManager is not active
      }
    }
  }
  
  lastPauseButtonState = currentButtonState;
}
