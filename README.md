# QCS - Quoin Control System

## Arduino Code
Quoin folder contains arduino code segmented into primary modules
- quoin - contains most global variables & definitions as well as setup, loop & general I/O routines
- time  - functions for setting and getting the time from a Real Time Clock chip
- sd    - functions for reading and writing data sets to the Arduino Ethernet's Micro-SD Card
- web   - functions for interpreting received http messages as well as sending json callbacks

## Web Interface
Web folder contains web front end to load content from arduino and display it nicely  
This was written by Kanshii and only tweaked by me