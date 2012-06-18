# QCS - Quoin Control System

At this point in time, the code is usable with web functions if the arduino is facing your local network
or is setup with a global ip, it will not work globally from Telstra's 3G service due to their use of a SuperNAT
that causes the Arduino to not receive a global address.

## Arduino Code
Quoin folder contains arduino code segmented into primary modules
- quoin - contains most global variables & definitions as well as setup, loop & general I/O routines
- time  - functions for setting and getting the time from a Real Time Clock chip
- sd    - functions for reading and writing data sets to the Arduino Ethernet's Micro-SD Card
- web   - functions for interpreting received http messages as well as sending json callbacks
- cosm  - functions to push data to cosm.com logging service for when pulling data though web functions wont work
          (such as when it's stuck behind a Telstra 3G SuperNAT)

## Web Interface
Web folder contains web front end to load content from arduino and display it nicely  
This was written by Kanshii and only tweaked by me