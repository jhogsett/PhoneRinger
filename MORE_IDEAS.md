## MORE IDEAS

- If the running time exceeds the value "99:99" switch from showing minutes & seconds to showing hours & minutes

- No longer seeing Max:X in the display

- Clean up the Line #3 display
    - use the format "A:X R:X E:X M:X"
        - A:X is active count
        - R:X is ringing count
        - E:X is enabled count
        - M:X is max concurrency
    - center justify the full string
    - the remaining 5 open chars on that line are for future use

- Keep Pin 13 as a System Status LED
    - when paused, it should have a fast blink instead of showing ringer power status
    - and return to its current behavior when not paused

- Use another pin as a "Ringer Power Up Control"
    - Use Pin 16 / A2
    - It should mostly follow what the system Status LED does - go HIGH if there are any energized relays
    - But it should also have a _hang time_ (settable in settings from 0 to 60 seconds) to dampen unneeded power cycles by staying on that much time after the System Status LED would turn off, and then resetting the hang time if a relay is energized again

- Menu Mode Tweaks
    - change "MENU MODE" to center-justified "* SETTINGS *"
    - sort the available settings by first letter
    - add a numeric label to the settings to make them easier to remember and recognize, like "1)Active Phones" "2)Call Timing" "3)Max Concurrent"
    - Make the Exit Menu item especially easily noticeable, perhaps with ASCII chars and an arrow
    



