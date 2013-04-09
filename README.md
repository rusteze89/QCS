# QCS - Quoin Control System

System designed for monitoring the voltage of a battery bank whilst also being able to control the on/off state of outputs.
Design choices are focused around the controller being connected to a webserver through a VPN over 3G and as such may not have very reliable connectivity, thus the controller must keep track of what data has and hasn't been sent within a reasonable timeframe.

## Arduino Code
This code is compiled and loaded onto an Arduino Ethernet using the Arduino IDE.
The Arduino code is segmented into 2 modules
- quoin - contains most global variables & definitions as well as setup, loop & general I/O routines
- web   - functions for interpreting received http messages as well as sending json callbacks with the latest data

## Web Interface
Web folder contains web front end to load content from arduino and display it nicely  
This was written by Kanshii and only tweaked by me

## Setup
in web/index.html replace my.device.net with the public address/ip that forwards to your device on port 80
