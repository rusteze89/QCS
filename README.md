# QCS - Quoin Control System

## Arduino Code
Quoin folder contains arduino code segmented into primary modules
- quoin - contains most global variables & definitions as well as setup and loop routines
- io    - contains functions for the input output operations for the control system
- time  - contains functions for setting and getting the time from a Real Time Clock chip
- web   - functions for interpreting received http messages as well as sending json callbacks

## Web Interface
Web folder contains web page to load content from arduino and display it nicely  
This code was written by Matt McDonald and only tweaked by me