/*
 * quoin.ino
 * base code for quoin control system
 * 
 * Modules:
 * * io.ino     contains functions for input/output
 * * time.ino   contains functions for communicating with RTC chip
 * * web.ino    contains functions to process and send jquery callback
 *
 * To Do:
 * * fix ethernet hangup - usually under high load / different clients
 * * test time code & implement data timestamping
 * * implement SD card storage
 */

#include <Ethernet.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>
#include "Wire.h"
#include <avr/pgmspace.h>
#include <utility/w5100.h>

#define VERSION       0.30

// Debugger enables
#define DEBUG         0     // debug operation by dumping to serial
#define DEBUG_MEM     0     // will make system unstable since it fills ram
#define DEBUG_WEB     0     // sends debug information to the web front end

// Functionality Switches
#define SD_EN         1     // enables SD card storage/retrieval of history
#define TIME_EN       1     // enables RTC time functions
#define WEB_EN        1     // enables web output functions

// General IO Pins used
#define APIN_ACPOWER  0     // analog pin for AC amps reading off inverter
#define APIN_BATTERY  1     // analog pin for battery voltage reading
#define APIN_SDA      5     // i2c SDA pin (analog 5)
#define APIN_SCL      6     // i2c SCL pin (analog 6)
#define PIN_RELAY1    2     // pin for power relay output
#define PIN_RELAY2    3     // pin for power relay output

// Constants for analog data
#define DATA_FREQ     5000  // 5 sec between data collections
#define DATA_INPUTS   2     // number of inputs being collected
#if SD_EN
  #define DATA_SET    1     // only need 1 data point when recording to SD
#else
  #define DATA_SET    50    // number of recent data points to keep
#endif

short data[3][DATA_SET];
unsigned long nextDataCheck;
byte  dataIndex;

// Setup
// performs once off setup operations on system startup
void setup()
{
  #if DEBUG
    Serial.begin(115200);   // start serial for Debugging
    Serial.println("\n\nQuoin Control System");
  #endif

  #if TIME_EN   // setup the real time clock
    setupTime();
  #endif
  #if SD_EN     // setup the sd card
    setupSD();
  #endif
  #if WEB_EN    // setup ethernet and web
    setupWeb();
  #endif

  // set outputs
  for (int i=0; i<8; i++)
    pinMode(i, OUTPUT);
  #if DEBUG
    Serial.println("initialization complete");
  #endif
}

// Loop
// Return Type: none
// Description: base code loop that manages system operation
void loop()
{
  // Check if it's time to update data
  if (millis() > nextDataCheck)
  {
    nextDataCheck += DATA_FREQ;
    checkAnalogData();
  }
  // if web output is enabled, respond to any web requests
  #if WEB_EN
    webCheck();
  #endif
}

//////////////////// IO OPERATIONS ////////////////////

// Check Data
// Description: reads analog inputs and stores the values
//              either onto SD or global vars
void checkAnalogData()
{
  // AC current mesurement
  int powerMax = 0;
  int val = 0;
  unsigned long tmeasure = millis() + 100;
  while (millis() < tmeasure)
  {
    val = analogRead(APIN_ACPOWER);

    // record the maximum sensor value
    if (val > powerMax)
      powerMax = val;
  }
  #if !SD_EN
    dataIndex = (dataIndex + 1) % DATA_SET;
  #endif
  data[APIN_ACPOWER][dataIndex] = powerMax;
  data[APIN_BATTERY][dataIndex] = map(analogRead(APIN_BATTERY), 0, 1024, 0, 1500);
  #if SD_EN
    writeSDanalog();
  #endif
}

// Toggle
// Description: Toggles the pin passed to it. Used to toggle relay outputs.
void toggle(byte pin)
{
  if (digitalRead(pin))
    digitalWrite(pin, LOW);
  else
    digitalWrite(pin, HIGH);
}

// Memory Test
// Return Type: short integer (2 bytes)
// Description: allocates ram, counting the number of bytes that were free
//    until the ram is full, returning that number and deallocating the ram used.
//    This function can cause the system to become unstable, particularly if
//    an interrupt routine occurs before memory is deallocated
short memoryTest()
{
  short byteCounter = 0;
  byte  *byteArray;

  while ((byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL) {
    byteCounter++;
    free(byteArray);
  }

  free(byteArray);
  return byteCounter;
}
