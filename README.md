# ArduinoBLE

This project facilitates BLE communication between an Arduino microcontroller and a mobile application made on Unity Game Engine. The Arduino will repeatedly send values from the MPR121 to the Unity APP where the values are checked against a certain threshold value. If the MPR121 values fall below the threshold, Unity will send a response back to the Arudino to determine if the Haptic motors will be activated.
