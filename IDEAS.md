## Ideas

- Use the LCD custom character feature to create an animated storm/twister/tornado that is cycled through its (8?) frames as part of the display update process during normal operation
    - Place the animating character alongside the CallStorm device name

- While in operational mode, so not in menu mode, when the encoder knob is turned, increase/decrease the current setting for _maximum enabled relays_ allowing a quick way to dial up or down device activity without entering the menu system
    - When the allow relays count is reduced, there is no need to immediately shut down any running ringers in excess of the max (for instance, if it's dialed back to zero, there's no need to shut all of them off immediately), just let them naturally finish their ringing cycle

- While in Menu Mode, _long pressing_ the encoder button should be a quick shortcut for "Save & Exit"

- While in Operation mode, _Long pressing_ the encoder button should activate  a secret feature that that sets all the options for the effect of **maximum chaos and ringing**: all relays enabled, all allowed to be concurrently running, with the minimum setting for call frequency.

---

## Development Roadmap

### Feature 1: Animated Storm Icon 🌪️
**Goal**: Add animated tornado/storm character alongside "CallStorm" branding

**Development Steps**:
1. **Design Custom Characters**
   - Create 4-6 frame animation of tornado/storm using LCD 5x8 pixel grid
   - Define custom character arrays in DisplayManager.h (LCD supports 8 custom chars)
   - Test individual frames on hardware to ensure readability

2. **Implement Animation Logic**
   - Add animation state tracking to DisplayManager class
   - Update character frame every 200-500ms during normal display updates
   - Integrate with existing showStatus() method

3. **Layout Integration**
   - Modify Line 1 layout: "CallStorm🌪️    12:34" (storm icon after name)
   - Ensure proper spacing and right-alignment of timer
   - Test with different timer lengths (1:23 vs 12:34)

4. **Memory Optimization**
   - Verify custom character storage doesn't impact RAM usage
   - Optimize frame switching to minimize LCD operations

**Estimated Effort**: 2-3 hours
**Files to Modify**: DisplayManager.h, DisplayManager.cpp

---

### Feature 2: Quick Relay Adjustment 🎛️
**Goal**: Encoder rotation in operation mode adjusts active relay count

**Development Steps**:
1. **Modify Encoder Handling**
   - Update main.cpp encoder event handling for operation mode
   - Add separate logic for CLOCKWISE/COUNTER_CLOCKWISE when !inMenu
   - Implement bounds checking (0-8 relays)

2. **Graceful Relay Reduction**
   - When reducing relay count, don't immediately stop active ringers
   - Update RingerManager to respect new limit for NEW calls only
   - Add "soft limit" logic that prevents new calls but allows current ones to finish

3. **Visual Feedback**
   - Show brief confirmation message on Line 2 when adjusted
   - Format: "Relays: 6→4" or "Max Relays: 4" for 2 seconds
   - Return to normal display automatically

4. **EEPROM Integration**
   - Save changes to activeRelaySetting immediately
   - Call saveSettingsToEEPROM() after each adjustment
   - Add debouncing to prevent excessive EEPROM writes

**Estimated Effort**: 2-3 hours
**Files to Modify**: main.cpp, RingerManager.cpp, DisplayManager.cpp

---

### Feature 3: Menu Long-Press Save & Exit 💾
**Goal**: Long-press encoder button in menu mode = quick save & exit

**Development Steps**:
1. **Extend Menu Event Handling**
   - Modify main.cpp menu logic to handle BUTTON_LONG_PRESS
   - Add logic to save settings and exit menu in one action
   - Works from any menu state (navigation or adjustment mode)

2. **User Feedback**
   - Show brief "Settings Saved!" message before returning to operation
   - Use Line 2 for confirmation message (2 second display)
   - Ensure smooth transition back to normal status display

3. **Code Consolidation**
   - Create helper function for "save and exit menu" logic
   - Reuse existing saveSettingsToEEPROM() functionality
   - Maintain consistency with existing menu behavior

**Estimated Effort**: 1 hour
**Files to Modify**: main.cpp

---

### Feature 4: "Maximum Chaos" Easter Egg 🎉
**Goal**: Long-press in operation mode = instant maximum activity

**Development Steps**:
1. **Define Chaos Settings**
   - Create constants for maximum chaos configuration:
     - activeRelaySetting = 8 (all relays enabled)
     - maxConcurrentSetting = 8 (all can ring simultaneously)
     - maxCallDelaySetting = 10 (minimum delay for maximum frequency)

2. **Implement Easter Egg**
   - Add BUTTON_LONG_PRESS handling in operation mode (!inMenu)
   - Apply chaos settings immediately without menu
   - Save to EEPROM for persistence

3. **Dramatic Visual Effect**
   - Show special message sequence:
     - Line 1: "CallStorm🌪️    XX:XX"
     - Line 2: "** CHAOS MODE **"
     - Line 3: "MAX EVERYTHING!"
     - Line 4: "🔥🔥🔥🔥🔥🔥🔥🔥" (or similar effect)
   - Display for 3 seconds before returning to normal

4. **Secret Documentation**
   - Add hidden comment in code explaining the feature
   - Consider adding subtle hint in startup message or menu

**Estimated Effort**: 1-2 hours
**Files to Modify**: main.cpp, DisplayManager.cpp

---

## Implementation Priority

**Phase 1 (Quick Wins)**:
- ✅ Feature 3: Menu Long-Press Save & Exit **COMPLETE!**
- ✅ Feature 4: Maximum Chaos Easter Egg **COMPLETE!**

**Phase 2 (Enhanced UX)**:
- ✅ Feature 2: Quick Relay Adjustment **COMPLETE!**

**Phase 3 (Polish)**:
- Feature 1: Animated Storm Icon

---

## Technical Considerations

**Memory Usage**:
- Current: RAM 71.2% (1459/2048 bytes), Flash 64.6% (19830/30720 bytes)
- Custom characters: ~40 bytes RAM, ~100 bytes Flash
- Additional logic: ~50-100 bytes RAM, ~200-400 bytes Flash
- **Total estimated impact**: +2-3% RAM, +1-2% Flash

**Code Quality**:
- Maintain String-free architecture
- Use existing global string buffer for all formatting
- Preserve EEPROM settings structure and versioning
- Keep atomic, testable changes with individual commits

**Testing Strategy**:
- Test each feature individually on hardware
- Verify EEPROM persistence after each feature
- Ensure no interference with existing functionality
- Validate memory usage remains within safe limits

