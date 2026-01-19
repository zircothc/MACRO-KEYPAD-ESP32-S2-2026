# MACRO-KEYPAD-ESP32-S2-2026
Macro keypad with web configurator

Simply 4x4 keypad and 3 RGB LEDS conected to a cheap Lolin s2 Mini (ESP32 S2)

#RED LEDS:
Keypad starts into CONFIG MODE. You can access to "Lolin_keypad" AP with your smartphone or PC.
Default AP password is "12345678".
Go to 192.168.4.1
You can change AP password and configure your macro keypad.

"SAVE CONFIGURATION" when done. Leds will be green.

#GREEN LEDS:
Keypad is ready to type. 

If you start keypad and dont want to configure simply press any key and leds will turn on green.
If you wnat to configure keypad, power off and power on again and leds will be red (ready to config).

#BLUE LED:
Keypad is sending text and keys stored on pressed key.

Libraries needed: Read code.

There are code tricks to become a Spanish (Spain) keyboard. Mostly all special characters are maped to key combinations.
If your language is not Spanish may be you need to change code.


