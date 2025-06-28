# Call Center Simulator Schematic

## Basic Wiring Diagram

```
Arduino Nano                    Relay Module                    Phone Circuit
┌─────────────┐                ┌──────────────┐               ┌─────────────┐
│             │                │              │               │             │
│  Pin 2   ───┼────────────────┤ IN1       NO ├───────────────┤ Ring (+)    │
│  Pin 3   ───┼────────────────┤ IN2       NO ├───────────────┤ Ring (+)    │  
│  Pin 4   ───┼────────────────┤ IN3       NO ├───────────────┤ Ring (+)    │
│  Pin 5   ───┼────────────────┤ IN4       NO ├───────────────┤ Ring (+)    │
│  Pin 6   ───┼────────────────┤ IN5       NO ├───────────────┤ Ring (+)    │
│  Pin 7   ───┼────────────────┤ IN6       NO ├───────────────┤ Ring (+)    │
│  Pin 8   ───┼────────────────┤ IN7       NO ├───────────────┤ Ring (+)    │
│  Pin 9   ───┼────────────────┤ IN8       NO ├───────────────┤ Ring (+)    │
│             │                │              │               │             │
│  GND     ───┼────────────────┤ GND       COM├───┐           │ Ring (-)    │
│  5V      ───┼────────────────┤ VCC          │   │           │             │
└─────────────┘                └──────────────┘   │           └─────────────┘
                                                   │
                                         Ring Voltage Source
                                              (~90V AC)
                                                   │
                                                   └───────────┐
                                                              │
                                             All phone Ring(-) ┘
```

## Components Needed

### Arduino Nano
- Microcontroller board
- USB programming capability
- Digital pins 2-9 used for relay control

### 8-Channel Relay Module
- Input voltage: 5V (from Arduino)
- Relay rating: Must handle telephone ring voltage (~90V AC, low current)
- Optoisolated inputs (recommended for safety)
- Active LOW or HIGH (adjust code accordingly)

### Ring Voltage Source
- Traditional telephone ring voltage: ~90V AC, 20Hz, low current
- Can be purchased as "telephone ring generator"
- Alternative: Use transformer to step up voltage (with proper safety measures)

### Safety Components (CRITICAL)
- Fuses on ring voltage circuit
- Isolation transformer
- Proper grounding
- Enclosure for high voltage components

## Safety Notes

⚠️ **DANGER - HIGH VOLTAGE**
- Telephone ring voltage can be lethal
- Use proper electrical safety procedures
- Test circuit with low voltage (12V) first
- Consider professional electrical consultation
- Always disconnect power when making connections

## Alternative Low-Voltage Testing

For initial testing and demonstration, you can use:
- 12V DC instead of ring voltage
- LEDs with current limiting resistors
- Buzzers or speakers
- Low voltage bells

This allows safe testing of the timing and logic before implementing the high-voltage ring circuit.

## Phone Connection Types

### Traditional Phones
- Connect to Ring (+) and Ring (-) terminals
- Some phones may need tip/ring connections
- Check phone documentation

### Modern Phones  
- May not respond to ring voltage
- Consider using external bells or ringers
- Vintage phones work best for this application

## Relay Selection

Choose relays rated for:
- Coil voltage: 5V DC (Arduino compatible)
- Contact voltage: ≥100V AC
- Contact current: ≥100mA (adequate for ring current)
- Isolation: High voltage isolation between coil and contacts
