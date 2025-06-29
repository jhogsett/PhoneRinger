#include "TelephoneRinger.h"
#include "RingerManager.h"
#include "DisplayManager.h"
#include "EncoderManager.h"

// Hardware pin definitions - Updated for your specific setup
const int RELAY_PINS[] = {5, 6, 7, 8, 9, 10, 11, 12}; // Digital pins 5-12 for 8-relay module
const int NUM_PHONES = 8;

// Configuration - Power Management
#define MAX_CONCURRENT_ACTIVE_PHONES 4  // Maximum phones that can be active simultaneously
                                         // Reduce this value for power supply testing (1-8)
                                         // Set to 1 for single-phone testing
                                         // Set to 8 to disable concurrent limiting

// Debug Mode - Set to true to silence normal operation serial output
#define DEBUG_ENCODER_MODE true  // Only show encoder events when true

// Menu System State
bool inMenu = false;
bool inAdjustmentMode = false;  // Track if we're adjusting a setting
int currentMenuItem = 0;
int maxConcurrentSetting = MAX_CONCURRENT_ACTIVE_PHONES;  // Local copy for menu editing
int activeRelaySetting = NUM_PHONES;  // Number of active relays (0-8)

// Static buffers for menu display - avoid String concatenation
char menuBuffer1[21];  // LCD line buffer (20 chars + null terminator)
char menuBuffer2[21];  // LCD line buffer
char menuBuffer3[21];  // LCD line buffer
char menuBuffer4[21];  // LCD line buffer

// Menu Items
enum MenuItems {
  MENU_CONCURRENT_LIMIT = 0,
  MENU_ACTIVE_RELAYS,  // Number of active relays (0-8)
  MENU_EXIT,
  MENU_ITEM_COUNT
};

const char* menuItemNames[] = {
  "Concurrent Limit",
  "Active Relays",  
  "Exit Menu"
};

// UI Hardware pins
const int ENCODER_PIN_A = 2;      // Encoder A
const int ENCODER_PIN_B = 3;      // Encoder B  
const int ENCODER_BUTTON = 4;     // Encoder button
const int PAUSE_BUTTON = A0;      // System pause button (moved from pin 13)
const int STATUS_LED = 13;        // System status LED (onboard LED)
// I2C pins A4 (SDA) and A5 (SCL) for 20x4 LCD display

// System state
bool systemPaused = false;
bool lastPauseButtonState = HIGH;  // Assuming pull-up resistor
bool pauseButtonPressed = false;   // Track if button was just pressed
unsigned long lastPauseDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// Status LED variables
bool statusLedState = false;
unsigned long lastStatusLedToggle = 0;
const unsigned long PAUSE_BLINK_INTERVAL = 100;  // 100ms = 10Hz toggle = 5Hz blink rate

// Global access to ringer manager for concurrent phone limit checking
RingerManager* globalRingerManager = nullptr;

// Create the system components
RingerManager ringerManager;
DisplayManager displayManager;
EncoderManager encoderManager;

// Function declarations
void checkPauseButton();
void updateStatusLED();
bool canStartNewCall();  // Check if a new call can start (respects concurrent limit)
void handleEncoderEvents();  // Handle rotary encoder input

void setup() {
  Serial.begin(115200);
  if (!DEBUG_ENCODER_MODE) {
    Serial.println(F("Call Center Simulator Starting..."));
    Serial.println(F("Hardware: 8-Relay Module + 20x4 LCD + Rotary Encoder + Pause Button"));
  }
  
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
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);  // Start with status LED off
  
  // Initialize the ringer manager with phone instances (using nullptr for config for now)
  ringerManager.initialize(RELAY_PINS, NUM_PHONES, nullptr, !DEBUG_ENCODER_MODE);
  
  // Set initial active relay count
  ringerManager.setActiveRelayCount(activeRelaySetting);
  
  // Set global pointer for concurrent phone limit checking
  globalRingerManager = &ringerManager;
  
  // Set callback for each phone to check concurrent limit
  ringerManager.setCanStartCallCallbackForAllPhones(canStartNewCall);
  
  // Initialize the display
  displayManager.initialize(!DEBUG_ENCODER_MODE);
  
  // Initialize the encoder
  encoderManager.initialize(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON, !DEBUG_ENCODER_MODE);
  
  if (!DEBUG_ENCODER_MODE) {
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
    Serial.println(F("- Status LED on pin 13 (onboard)"));
    Serial.println(F("- 20x4 LCD on I2C (A4/A5)"));
    Serial.print(F("- Concurrent phone limit: "));
    Serial.print(maxConcurrentSetting);
    Serial.println(F(" phones maximum"));
    Serial.println(F("Press pause button (pin A0) to stop/start all activity"));
    Serial.println(F("Status LED: ON=phones ringing, Fast Blink=paused, OFF=idle"));
    
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
  } else {
    // Silent mode - just do the relay test without serial output
    for (int i = 0; i < NUM_PHONES; i++) {
      digitalWrite(RELAY_PINS[i], LOW);  // LOW = active for active-LOW modules
      delay(200);
      digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW modules
      delay(100);
    }
    Serial.println(F("=== ENCODER DEBUG MODE - Only encoder events will be shown ==="));
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check pause button
  checkPauseButton();
  
  // Handle encoder events
  handleEncoderEvents();
  
  // Only step the ringer manager if not paused AND we have active relays
  if (!systemPaused && activeRelaySetting > 0) {
    ringerManager.step(currentTime);
  }
  
  // If active relay count changed, update RingerManager
  static int lastActiveRelayCount = activeRelaySetting;
  if (lastActiveRelayCount != activeRelaySetting) {
    ringerManager.setActiveRelayCount(activeRelaySetting);
    lastActiveRelayCount = activeRelaySetting;
    if (!DEBUG_ENCODER_MODE) {
      Serial.print(F("Active relays changed to: "));
      Serial.println(activeRelaySetting);
    }
  }
  
  // Update display (only when not in menu mode)
  if (!inMenu) {
    displayManager.update(currentTime, systemPaused, &ringerManager, maxConcurrentSetting);
  }
  
  // Update status LED
  updateStatusLED();
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void checkPauseButton() {
  bool currentButtonState = digitalRead(PAUSE_BUTTON);
  
  // Debug output every 5000ms to see button state (only if not in encoder debug mode)
  if (!DEBUG_ENCODER_MODE) {
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime > 5000) {
      Serial.print(F("Pause Button: "));
      Serial.print(currentButtonState ? "HIGH" : "LOW");
      Serial.print(F(" | Paused: "));
      Serial.println(systemPaused ? "YES" : "NO");
      lastDebugTime = millis();
    }
  }
  
  // Simple debounce: if button state changed, reset debounce timer
  if (currentButtonState != lastPauseButtonState) {
    lastPauseDebounce = millis();
    pauseButtonPressed = false; // Reset press flag
    if (!DEBUG_ENCODER_MODE) {
      Serial.print(F("Button changed: "));
      Serial.print(lastPauseButtonState ? "HIGH" : "LOW");
      Serial.print(F(" -> "));
      Serial.println(currentButtonState ? "HIGH" : "LOW");
    }
  }
  
  // Check if enough time has passed for debounce
  if ((millis() - lastPauseDebounce) > DEBOUNCE_DELAY) {
    // If button is pressed (LOW) and we haven't processed this press yet
    if (currentButtonState == LOW && !pauseButtonPressed) {
      pauseButtonPressed = true; // Mark as processed
      if (!DEBUG_ENCODER_MODE) {
        Serial.println(F("*** PAUSE BUTTON PRESSED - TOGGLING STATE ***"));
      }
      
      // Toggle pause state
      systemPaused = !systemPaused;
      
      if (systemPaused) {
        if (!DEBUG_ENCODER_MODE) {
          Serial.println(F("*** SYSTEM PAUSED - Stopping all activity ***"));
        }
        // Stop all calls and turn off all relays immediately
        ringerManager.stopAllCalls();
        for (int i = 0; i < NUM_PHONES; i++) {
          digitalWrite(RELAY_PINS[i], HIGH); // HIGH = inactive for active-LOW relay modules
        }
        displayManager.showPauseMessage();
      } else {
        if (!DEBUG_ENCODER_MODE) {
          Serial.println(F("*** SYSTEM RESUMED - Activity will restart ***"));
        }
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

void updateStatusLED() {
  unsigned long currentTime = millis();
  
  if (systemPaused) {
    // System is paused - blink at 5 Hz (100ms toggle interval)
    if (currentTime - lastStatusLedToggle >= PAUSE_BLINK_INTERVAL) {
      statusLedState = !statusLedState;
      digitalWrite(STATUS_LED, statusLedState ? HIGH : LOW);
      lastStatusLedToggle = currentTime;
    }
  } else {
    // System is running - solid ON if any phone is ringing, OFF if idle
    int ringingPhones = ringerManager.getRingingPhoneCount();
    bool shouldBeOn = (ringingPhones > 0);
    
    // Only update LED if state needs to change (avoid unnecessary writes)
    if (statusLedState != shouldBeOn) {
      statusLedState = shouldBeOn;
      digitalWrite(STATUS_LED, statusLedState ? HIGH : LOW);
    }
  }
}

// Check if a new call can start (respects concurrent phone limit AND active relay count)
bool canStartNewCall() {
  if (globalRingerManager == nullptr) {
    return false;  // Safety check
  }
  
  // No calls allowed if active relays is 0
  if (activeRelaySetting == 0) {
    return false;
  }
  
  int currentActivePhones = globalRingerManager->getActiveCallCount();
  bool canStart = currentActivePhones < maxConcurrentSetting;
  
  // Optional debug output (can be commented out for production)
  if (!canStart && !DEBUG_ENCODER_MODE) {
    Serial.print(F("Concurrent limit reached: "));
    Serial.print(currentActivePhones);
    Serial.print(F("/"));
    Serial.print(maxConcurrentSetting);
    Serial.println(F(" phones already active"));
  }
  
  return canStart;
}

// Handle rotary encoder input
void handleEncoderEvents() {
  EncoderManager::EncoderEvent event = encoderManager.update();
  
  if (event != EncoderManager::NONE) {
    Serial.print(F("Encoder Event: "));
    Serial.println(encoderManager.getEventString(event));
    
    // Handle button press for menu toggle/selection
    if (event == EncoderManager::BUTTON_PRESS) {
      if (!inMenu) {
        inMenu = true;
        inAdjustmentMode = false;
        currentMenuItem = 0;  // Start at first menu item
        Serial.println(F("*** ENTERING MENU MODE ***"));
        Serial.print(F("Menu Item: "));
        Serial.println(menuItemNames[currentMenuItem]);
        displayManager.showMessage("MENU MODE", menuItemNames[currentMenuItem], 
                                   "Turn: Navigate", "Press: Select/Exit");
      } else {
        // In menu - handle selection
        if (inAdjustmentMode) {
          // We're adjusting a setting - save and return to menu navigation
          Serial.println(F("*** SAVING SETTING & RETURNING TO MENU ***"));
          Serial.print(F("Concurrent limit saved as: "));
          Serial.println(maxConcurrentSetting);
          inAdjustmentMode = false;  // Exit adjustment mode but stay in menu
          // Show the current menu item again
          displayManager.showMessage("MENU MODE", menuItemNames[currentMenuItem], 
                                     "Turn: Navigate", "Press: Select/Exit");
        } else {
          // Not in adjustment mode - handle menu selection
          switch (currentMenuItem) {
            case MENU_CONCURRENT_LIMIT:
              Serial.println(F("*** SELECTED: Concurrent Limit ***"));
              Serial.print(F("Current value: "));
              Serial.println(maxConcurrentSetting);
              inAdjustmentMode = true;  // Enter adjustment mode
              snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", maxConcurrentSetting);
              displayManager.showMessage("Concurrent Limit", 
                                         menuBuffer2,
                                         "Turn: Adjust (1-8)", "Press: Save & Back");
              break;
              
            case MENU_ACTIVE_RELAYS:
              Serial.println(F("*** SELECTED: Active Relays ***"));
              Serial.print(F("Current value: "));
              Serial.println(activeRelaySetting);
              inAdjustmentMode = true;  // Enter adjustment mode
              snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
              displayManager.showMessage("Active Relays", 
                                         menuBuffer2,
                                         "Turn: Adjust (0-8)", "Press: Save & Back");
              break;
              
            case MENU_EXIT:
              inMenu = false;
              inAdjustmentMode = false;
              Serial.println(F("*** EXITING MENU MODE ***"));
              displayManager.showStatus(&ringerManager, systemPaused, maxConcurrentSetting);
              break;
          }
        }
      }
      return;
    }
    
    // Handle rotation events
    if (inMenu) {
      switch (event) {
        case EncoderManager::CLOCKWISE:
          if (inAdjustmentMode && currentMenuItem == MENU_CONCURRENT_LIMIT && maxConcurrentSetting < 8) {
            // If we're adjusting concurrent limit, increment the value
            maxConcurrentSetting++;
            Serial.print(F("MENU: Concurrent limit increased to "));
            Serial.println(maxConcurrentSetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", maxConcurrentSetting);
            displayManager.showMessage("Concurrent Limit", 
                                       menuBuffer2,
                                       "Turn: Adjust (1-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_ACTIVE_RELAYS && activeRelaySetting < 8) {
            // If we're adjusting active relay count, increment the value
            activeRelaySetting++;
            Serial.print(F("MENU: Active relays increased to "));
            Serial.println(activeRelaySetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
            displayManager.showMessage("Active Relays", 
                                       menuBuffer2,
                                       "Turn: Adjust (0-8)", "Press: Save & Back");
          } else if (!inAdjustmentMode) {
            // Navigate to next menu item
            currentMenuItem = (currentMenuItem + 1) % MENU_ITEM_COUNT;
            Serial.print(F("MENU: Navigate to "));
            Serial.println(menuItemNames[currentMenuItem]);
            displayManager.showMessage("MENU MODE", menuItemNames[currentMenuItem], 
                                       "Turn: Navigate", "Press: Select/Exit");
          }
          break;
          
        case EncoderManager::COUNTER_CLOCKWISE:
          if (inAdjustmentMode && currentMenuItem == MENU_CONCURRENT_LIMIT && maxConcurrentSetting > 1) {
            // If we're adjusting concurrent limit, decrement the value
            maxConcurrentSetting--;
            Serial.print(F("MENU: Concurrent limit decreased to "));
            Serial.println(maxConcurrentSetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", maxConcurrentSetting);
            displayManager.showMessage("Concurrent Limit", 
                                       menuBuffer2,
                                       "Turn: Adjust (1-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_ACTIVE_RELAYS && activeRelaySetting > 0) {
            // If we're adjusting active relay count, decrement the value
            activeRelaySetting--;
            Serial.print(F("MENU: Active relays decreased to "));
            Serial.println(activeRelaySetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
            displayManager.showMessage("Active Relays", 
                                       menuBuffer2,
                                       "Turn: Adjust (0-8)", "Press: Save & Back");
          } else if (!inAdjustmentMode) {
            // Navigate to previous menu item
            currentMenuItem = (currentMenuItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
            Serial.print(F("MENU: Navigate to "));
            Serial.println(menuItemNames[currentMenuItem]);
            displayManager.showMessage("MENU MODE", menuItemNames[currentMenuItem], 
                                       "Turn: Navigate", "Press: Select/Exit");
          }
          break;
          
        case EncoderManager::BUTTON_LONG_PRESS:
          Serial.println(F("Long press: Reset concurrent limit to default"));
          maxConcurrentSetting = MAX_CONCURRENT_ACTIVE_PHONES;
          snprintf(menuBuffer2, sizeof(menuBuffer2), "Reset to: %d", maxConcurrentSetting);
          displayManager.showMessage("Concurrent Limit", 
                                     menuBuffer2,
                                     "Turn: Adjust (1-8)", "Press: Save & Back");
          break;
          
        default:
          break;
      }
    } else {
      // Not in menu - normal operation
      switch (event) {
        case EncoderManager::CLOCKWISE:
          Serial.println(F("NORMAL: Would increment setting"));
          break;
          
        case EncoderManager::COUNTER_CLOCKWISE:
          Serial.println(F("NORMAL: Would decrement setting"));
          break;
          
        case EncoderManager::BUTTON_LONG_PRESS:
          Serial.println(F("Long press: Would reset to defaults"));
          break;
          
        default:
          break;
      }
    }
  }
}
