Li-Fi Security System
Overview
This repository contains the source code and documentation for a Li-Fi based security system. The system is designed to monitor and respond to various environmental dangers using a sender platform (TM4C123GH6PM) connected to three sensors (Fume Sensor, Ultrasonic Sensor, and Magnetic Sensor). When any of the sensors detect a danger, the system sends alerts through both light communication (Li-Fi) to a receiver platform and Bluetooth to a mobile phone.

Features
Sensor Integration:

Fume Sensor: Detects gas or smoke related to fire.
Ultrasonic Sensor: Monitors changes in distance, potentially indicating intrusion.
Magnetic Sensor: Detects changes in magnetic fields, such as the opening of a door.
Alerting Mechanisms:

Li-Fi Communication (Light):
Alarms represented by a flashing lamp, buzzer, and LCD display on the receiver platform.
Bluetooth Communication:
Sends alerts to a mobile phone using Bluetooth technology, displaying relevant sentences.
Control Operations:

Start/stop operations controlled by two pushbuttons.
The system can be closed at any time.
Alarm Handling:

Continuous sound alerts until a mute pushbutton is pressed.
Muting the alarms turns off all alerts after a 5-second delay, while keeping the system powered on.
