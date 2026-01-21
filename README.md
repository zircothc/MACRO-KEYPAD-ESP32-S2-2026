# MACRO-KEYPAD-ESP32-S2-2026
Macro keypad with web configurator
![macro_kp](https://github.com/user-attachments/assets/aa050ffb-19e1-4d30-b1a6-2da01ade3e39)

4x4 keypad

1 RGB neopixel style LED

Lolin S2 Mini (ESP32 S2)

This is a basic but powerfull programmable macro keypad. Access keypad over wifi, configure your keys, long text, delays into 16 slots, save and run.

Arduino code.
<img width="1180" height="640" alt="pantalla" src="https://github.com/user-attachments/assets/c9917541-544d-490d-ab03-717039fdcfa6" />

# RED LED:
Keypad starts into CONFIG MODE. You can access to "Lolin_Keypad" AP with your smartphone or PC.

Default AP password is "12345678".

Go to 192.168.4.1

You can change AP password and configure your macro keypad.

"SAVE CONFIGURATION" when done. Led will turn green.

# GREEN LED:
Keypad is ready to type. 

If you power on keypad and dont want to configure, you want to use it, press any key and led will turn  green.

If you need to configure keypad, power off and power on again. Led will be red (ready to config) again.

# BLUE LED:
Keypad is sending text and keys stored on pressed key.

Libraries needed: Read code. I will update urls.

There are code tricks to become a Spanish (Spain) keyboard. Mostly all special characters are converted to key combinations. Not all :(

If your language is not Spanish may be you need to clean or change the code.



