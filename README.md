# Call Center Simulator - Arduino Nano Project

This project creates a realistic call center simulator using an Arduino Nano to control 8 relays that switch telephone ring voltage to create the effect of multiple incoming calls.

This is a PlatformIO project for VS Code with proper project structure and IntelliSense support.

## Hardware Requirements

- Arduino Nano
- 8x Relay module (pins 5-12)
- 8x Traditional telephone ringers or phones  
- Ring voltage source (~90V AC, low current)
- 20x4 LCD display with I2C backpack (A4/A5)
- Rotary encoder with button (pins 2, 3, 4)
- Pause button (pin 13)
- Appropriate safety components (fuses, isolation transformers)

## Pin Assignments

- **Digital pins 5-12**: 8-channel relay module control outputs
- **Digital pins 2-3**: Rotary encoder A & B
- **Digital pin 4**: Rotary encoder button
- **Digital pin 13**: System pause button
- **Analog pins A4/A5**: I2C for 20x4 LCD display (SDA/SCL)
- Each relay controls one telephone line

## Features

- **Realistic Ring Timing**: Uses proper U.S. telephone system timing (2 seconds on, 4 seconds off)
- **Variable Ring Count**: Each call rings 1-8 times randomly
- **Simulated Call Answering**: Multi-ring calls have a 70% chance of having the final ring cut short (simulating someone answering)
- **Random Call Timing**: Random delays between calls (5-30 seconds) to create realistic call center atmosphere
- **Asynchronous Operation**: All timing handled asynchronously using millis() for precise timing
- **20x4 LCD Display**: Real-time status showing active calls, ringing phones, and system state
- **System Pause**: Emergency pause button stops all relay activity instantly
- **Status Monitoring**: Both LCD display and Serial output show call activity and statistics
- **Future-Ready Architecture**: Modular design ready for additional features

## Safety Warnings

⚠️ **HIGH VOLTAGE WARNING**: Telephone ring voltage is typically around 90V AC. Use appropriate safety measures:
- Use properly rated relays
- Include fuses and circuit protection
- Use isolation transformers
- Follow electrical safety guidelines
- Test with low voltage first

## Software Architecture

### TelephoneRinger Class
- Handles individual phone ringing state machine
- Manages ring timing and sequences
- Simulates call answering behavior

### RingerManager Class  
- Manages pool of TelephoneRinger instances
- Coordinates timing across all phones
- Provides status monitoring and control

## Project Structure

```
PhoneRinger/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main Arduino sketch
│   ├── TelephoneRinger.cpp # Individual phone ringer implementation
│   └── RingerManager.cpp   # Phone pool manager implementation
├── include/
│   ├── TelephoneRinger.h   # Individual phone ringer header
│   └── RingerManager.h     # Phone pool manager header
├── README.md               # This file
└── WIRING.md              # Hardware wiring diagrams
```

## Setup

1. Open the project in VS Code with PlatformIO extension installed
2. PlatformIO will automatically detect the project configuration
3. Use `Ctrl+Shift+P` → "PlatformIO: Build" to compile
4. Use `Ctrl+Shift+P` → "PlatformIO: Upload" to flash to Arduino
5. Use `Ctrl+Shift+P` → "PlatformIO: Serial Monitor" to view output

## Usage

1. Build and upload the code to your Arduino Nano
2. Connect 8-channel relay module to pins 5-12
3. Connect 20x4 LCD display to I2C pins (A4/A5)  
4. Connect rotary encoder to pins 2, 3, 4
5. Connect pause button to pin 13 (with pull-up resistor)
6. Connect telephone ringers through relays
7. Open PlatformIO Serial Monitor (9600 baud) to see activity
8. The LCD will show real-time status of all phone lines
9. Press the pause button (pin 13) to immediately stop/start all activity
10. The system will automatically start generating calls

## LCD Display Information

The 20x4 LCD shows:
- **Line 1**: System title and uptime
- **Line 2**: Active calls, ringing phones, total phones  
- **Line 3**: Visual status of all 8 phone lines (R1=Ringing, A1=Active, --=Idle)
- **Line 4**: System status and pause button reminder

## Controls

- **Pause Button (Pin 13)**: Immediately stops all relay activity when pressed, press again to resume
- **Rotary Encoder (Pins 2,3,4)**: Ready for future menu navigation (not yet implemented)

## Wiring Example

```
Arduino Pin 2 → Relay 1 Control → Phone 1 Ring Circuit
Arduino Pin 3 → Relay 2 Control → Phone 2 Ring Circuit
...
Arduino Pin 9 → Relay 8 Control → Phone 8 Ring Circuit
```

## Customization

You can modify the timing constants in `include/TelephoneRinger.h`:
- `RING_ON_DURATION`: How long each ring lasts
- `RING_OFF_DURATION`: Silence between rings  
- `MIN_WAIT_TIME` / `MAX_WAIT_TIME`: Delay between call attempts
- `SHORT_RING_DURATION`: Duration of cut-short "answered" rings

## Serial Output

The system provides real-time status updates showing:
- When calls start and how many rings
- Overall statistics (active calls, ringing phones)
- Visual representation of phone states

Example output:
```
Phone on pin 3 starting call: 4 rings (final ring cut short)
Status: 3 active calls, 2 phones ringing out of 8 total phones
Phones: .R.A.R.. (R=Ringing, A=Active, .=Idle)
```
