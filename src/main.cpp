#include "TelephoneRinger.h"
#include "RingerManager.h"
#include "DisplayManager.h"
#include "EncoderManager.h"
#include "SettingsManager.h"
#include "RandomSeed.h"

// Hardware pin definitions - Updated for your specific setup
const int RELAY_PINS[] = {5, 6, 7, 8, 9, 10, 11, 12}; // Digital pins 5-12 for 8-relay module
const int NUM_PHONES = 8;

// Configuration - Power Management
#define MAX_CONCURRENT_ACTIVE_PHONES 4  // Maximum phones that can be active simultaneously
                                         // Reduce this value for power supply testing (1-8)
                                         // Set to 1 for single-phone testing
                                         // Set to 8 to disable concurrent limiting

// Maximum Chaos Mode Settings - The ultimate CallStorm 2000 experience!
#define CHAOS_ACTIVE_RELAYS 8        // All relays enabled
#define CHAOS_MAX_CONCURRENT 8       // All phones can ring simultaneously  
#define CHAOS_MIN_CALL_DELAY 10      // Minimum delay = maximum frequency

// Debug Mode - Set to true to silence normal operation serial output
#define DEBUG_ENCODER_MODE true  // Only show encoder events when true

// Menu System State
bool inMenu = false;
bool inAdjustmentMode = false;  // Track if we're adjusting a setting
int currentMenuItem = 0;
int maxConcurrentSetting = MAX_CONCURRENT_ACTIVE_PHONES;  // Local copy for menu editing
int activeRelaySetting = NUM_PHONES;  // Number of active relays (0-8)
int maxCallDelaySetting = 30;  // Maximum delay between calls in seconds (10-1000, increments of 10)

// Static buffers for menu display - avoid String concatenation
char menuBuffer1[21];  // LCD line buffer (20 chars + null terminator)
char menuBuffer2[21];  // LCD line buffer
char menuBuffer3[21];  // LCD line buffer
char menuBuffer4[21];  // LCD line buffer

// Menu Items
enum MenuItems {
  MENU_CONCURRENT_LIMIT = 0,
  MENU_ACTIVE_RELAYS,  // Number of active relays (0-8)
  MENU_CALL_FREQUENCY, // Maximum delay between calls (10-1000 seconds)
  MENU_EXIT,
  MENU_ITEM_COUNT
};

const char* menuItemNames[] = {
  "Max Concurrent",
  "Active Phones",  
  "Call Timing",
  "Exit Menu"
};

// UI Hardware pins
const int ENCODER_PIN_A = 3;      // Encoder A
const int ENCODER_PIN_B = 2;      // Encoder B  
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
void loadSettingsFromEEPROM();
void saveSettingsToEEPROM();
void activateMaximumChaos(); // 🌪️ Maximum Chaos Easter Egg!
void showRelayAdjustmentFeedback(); // Show brief relay adjustment confirmation
void saveAndExitMenu(); // 💾 Menu Long-Press: Save & Exit

void setup() {
  Serial.begin(115200);
  if (!DEBUG_ENCODER_MODE) {
    Serial.println(F("CallStorm Simulator Starting..."));
    Serial.println(F("Hardware: 8-Relay Module + 20x4 LCD + Rotary Encoder + Pause Button"));
  }
  
  // Seed the random number generator with atmospheric noise using improved randomizer
  RandomSeed<A1> atmosphericRNG;  // Use A1 for dedicated random seeding (A0 is pause button)
  atmosphericRNG.randomize();
  
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

  // Load settings from EEPROM (before applying them)
  loadSettingsFromEEPROM();
  
  // Set initial active relay count from loaded settings
  ringerManager.setActiveRelayCount(activeRelaySetting);
  
  // Set global pointer for concurrent phone limit checking
  globalRingerManager = &ringerManager;
  
  // Set callback for each phone to check concurrent limit
  ringerManager.setCanStartCallCallbackForAllPhones(canStartNewCall);
  
  // Initialize the display
  displayManager.initialize(!DEBUG_ENCODER_MODE);
  
  // Initialize the encoder
  encoderManager.initialize(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON, !DEBUG_ENCODER_MODE);
  
  // Load settings from EEPROM
  loadSettingsFromEEPROM();
  
  if (!DEBUG_ENCODER_MODE) {
    Serial.println(F("CallStorm Simulator Ready!"));
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
        // Turn off all relays immediately but don't stop the call state machines
        // This preserves timing so calls remain unsynchronized when resumed
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
          
          // Save all settings to EEPROM when any setting is changed
          saveSettingsToEEPROM();
          
          Serial.print(F("Settings saved: Concurrent="));
          Serial.print(maxConcurrentSetting);
          Serial.print(F(", Active="));
          Serial.print(activeRelaySetting);
          Serial.print(F(", Frequency="));
          Serial.println(maxCallDelaySetting);
          
          inAdjustmentMode = false;  // Exit adjustment mode but stay in menu
          // Show the current menu item again
          displayManager.showMessage("MENU MODE", menuItemNames[currentMenuItem], 
                                     "Turn: Navigate", "Press: Select/Exit");
        } else {
          // Not in adjustment mode - handle menu selection
          switch (currentMenuItem) {
            case MENU_CONCURRENT_LIMIT:
              Serial.println(F("*** SELECTED: Max Concurrent ***"));
              Serial.print(F("Current value: "));
              Serial.println(maxConcurrentSetting);
              inAdjustmentMode = true;  // Enter adjustment mode
              snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", maxConcurrentSetting);
              displayManager.showMessage("Max Concurrent", 
                                         menuBuffer2,
                                         "Turn: Adjust (1-8)", "Press: Save & Back");
              break;
              
            case MENU_ACTIVE_RELAYS:
              Serial.println(F("*** SELECTED: Active Phones ***"));
              Serial.print(F("Current value: "));
              Serial.println(activeRelaySetting);
              inAdjustmentMode = true;  // Enter adjustment mode
              snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
              displayManager.showMessage("Active Phones", 
                                         menuBuffer2,
                                         "Turn: Adjust (0-8)", "Press: Save & Back");
              break;
              
            case MENU_CALL_FREQUENCY:
              Serial.println(F("*** SELECTED: Call Timing ***"));
              Serial.print(F("Current value: "));
              Serial.print(maxCallDelaySetting);
              Serial.println(F(" seconds"));
              inAdjustmentMode = true;  // Enter adjustment mode
              snprintf(menuBuffer2, sizeof(menuBuffer2), "Max: %ds", maxCallDelaySetting);
              displayManager.showMessage("Call Timing", 
                                         menuBuffer2,
                                         "Turn: +/-10s (10-1000)", "Press: Save & Back");
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
            displayManager.showMessage("Max Concurrent", 
                                       menuBuffer2,
                                       "Turn: Adjust (1-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_ACTIVE_RELAYS && activeRelaySetting < 8) {
            // If we're adjusting active relay count, increment the value
            activeRelaySetting++;
            Serial.print(F("MENU: Active relays increased to "));
            Serial.println(activeRelaySetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
            displayManager.showMessage("Active Phones", 
                                       menuBuffer2,
                                       "Turn: Adjust (0-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_CALL_FREQUENCY && maxCallDelaySetting < 1000) {
            // If we're adjusting call frequency, increment the value
            maxCallDelaySetting += 10;
            Serial.print(F("MENU: Call frequency increased to "));
            Serial.print(maxCallDelaySetting);
            Serial.println(F(" seconds"));
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Max: %ds", maxCallDelaySetting);
            displayManager.showMessage("Call Timing", 
                                       menuBuffer2,
                                       "Turn: +/-10s (10-1000)", "Press: Save & Back");
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
            displayManager.showMessage("Max Concurrent", 
                                       menuBuffer2,
                                       "Turn: Adjust (1-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_ACTIVE_RELAYS && activeRelaySetting > 0) {
            // If we're adjusting active relay count, decrement the value
            activeRelaySetting--;
            Serial.print(F("MENU: Active relays decreased to "));
            Serial.println(activeRelaySetting);
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Setting: %d", activeRelaySetting);
            displayManager.showMessage("Active Phones", 
                                       menuBuffer2,
                                       "Turn: Adjust (0-8)", "Press: Save & Back");
          } else if (inAdjustmentMode && currentMenuItem == MENU_CALL_FREQUENCY && maxCallDelaySetting > 10) {
            // If we're adjusting call frequency, decrement the value
            maxCallDelaySetting -= 10;
            Serial.print(F("MENU: Call frequency decreased to "));
            Serial.print(maxCallDelaySetting);
            Serial.println(F(" seconds"));
            snprintf(menuBuffer2, sizeof(menuBuffer2), "Max: %ds", maxCallDelaySetting);
            displayManager.showMessage("Call Timing", 
                                       menuBuffer2,
                                       "Turn: +/-10s (10-1000)", "Press: Save & Back");
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
          // Menu Long-Press: Save & Exit from any menu state
          Serial.println(F("Menu Long-Press: Save & Exit"));
          saveAndExitMenu();
          break;
          
        default:
          break;
      }
    } else {
      // Not in menu - normal operation mode
      // Encoder rotation adjusts active relay count directly
      switch (event) {
        case EncoderManager::CLOCKWISE:
          if (activeRelaySetting < 8) {
            activeRelaySetting++;
            Serial.print(F("OPERATION: Active relays increased to "));
            Serial.println(activeRelaySetting);
            // Show brief +1 feedback and save to EEPROM
            displayManager.showRelayAdjustmentDirection(activeRelaySetting, true);
            saveSettingsToEEPROM();
          } else {
            Serial.println(F("OPERATION: Already at maximum relays (8)"));
          }
          break;
          
        case EncoderManager::COUNTER_CLOCKWISE:
          if (activeRelaySetting > 0) {
            activeRelaySetting--;
            Serial.print(F("OPERATION: Active relays decreased to "));
            Serial.println(activeRelaySetting);
            // Show brief -1 feedback and save to EEPROM
            displayManager.showRelayAdjustmentDirection(activeRelaySetting, false);
            saveSettingsToEEPROM();
          } else {
            Serial.println(F("OPERATION: Already at minimum relays (0)"));
          }
          break;
          
        case EncoderManager::BUTTON_LONG_PRESS:
          activateMaximumChaos();
          break;
          
        default:
          break;
      }
    }
  }
}

// Function to load settings from EEPROM
void loadSettingsFromEEPROM() {
  Settings settings;
  if (SettingsManager::loadSettings(settings)) {
    // Successfully loaded settings
    maxConcurrentSetting = settings.maxConcurrent;
    activeRelaySetting = settings.activeRelays;  
    maxCallDelaySetting = settings.maxCallDelay;
    Serial.println(F("Settings loaded from EEPROM"));
    Serial.print(F("  Concurrent Limit: ")); Serial.println(maxConcurrentSetting);
    Serial.print(F("  Active Relays: ")); Serial.println(activeRelaySetting);
    Serial.print(F("  Call Frequency: ")); Serial.println(maxCallDelaySetting);
  } else {
    // Failed to load - using defaults (already set)
    Serial.println(F("Using default settings (EEPROM invalid/empty)"));
    saveSettingsToEEPROM(); // Save defaults to EEPROM
  }
}

// Function to save settings to EEPROM
void saveSettingsToEEPROM() {
  Settings settings;
  settings.maxConcurrent = maxConcurrentSetting;
  settings.activeRelays = activeRelaySetting;
  settings.maxCallDelay = maxCallDelaySetting;
  
  if (SettingsManager::saveSettings(settings)) {
    Serial.println(F("Settings saved to EEPROM"));
  } else {
    Serial.println(F("Failed to save settings to EEPROM"));
  }
}

// 🌪️ MAXIMUM CHAOS MODE - The ultimate CallStorm 2000 experience!
void activateMaximumChaos() {
  // Apply maximum chaos settings silently
  maxConcurrentSetting = CHAOS_MAX_CONCURRENT;
  activeRelaySetting = CHAOS_ACTIVE_RELAYS;
  maxCallDelaySetting = CHAOS_MIN_CALL_DELAY;
  
  // Re-seed with maximum entropy for true chaos
  RandomSeed<A1> chaosRNG;
  chaosRNG.randomize();
  
  // Save chaos settings to EEPROM for persistence
  saveSettingsToEEPROM();
  
  // Display dramatic chaos message (the only visible feedback)
  displayManager.showChaosMessage();
}

// 🎛️ QUICK RELAY ADJUSTMENT - Show brief feedback for encoder-based relay adjustment
void showRelayAdjustmentFeedback() {
  // Display brief confirmation message
  displayManager.showRelayAdjustmentMessage(activeRelaySetting);
}

// 💾 MENU LONG-PRESS SAVE & EXIT - Quick save and return to operation
void saveAndExitMenu() {
  // Save all current settings to EEPROM
  saveSettingsToEEPROM();
  
  // Show brief confirmation message
  displayManager.showSaveExitMessage();
  
  // Exit menu mode and return to normal operation
  inMenu = false;
  inAdjustmentMode = false;
  
  Serial.println(F("*** MENU LONG-PRESS: SAVED & EXITED ***"));
  Serial.print(F("Settings saved: Concurrent="));
  Serial.print(maxConcurrentSetting);
  Serial.print(F(", Active="));
  Serial.print(activeRelaySetting);
  Serial.print(F(", Frequency="));
  Serial.println(maxCallDelaySetting);
  
  // The normal status display will resume automatically via the main loop
}
