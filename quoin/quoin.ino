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
#define DEBUG_MEM     0     // this will make system unstable since it fills ram
#define DEBUG_WEB     0     // sends debug information to the web front end

// Functionality Switches
#define SD_EN         0     // enables SD card storage/retrieval of history
#define TIME_EN       1     // enables RTC time functions
#define WEB_EN        1     // enables web output functions

// General IO Pins used
#define APIN_ACPOWER  0     // analog pin for AC amps reading off inverter
#define APIN_BATTERY  1     // analog pin for battery voltage reading
#define APIN_SOLAR    2     // analog pin for solar voltage reading
#define APIN_SDA      5     // i2c SDA pin (analog 5)
#define APIN_SCL      6     // i2c SCL pin (analog 6)
#define PIN_RELAY1    2     // pin for power relay output
#define PIN_RELAY2    3     // pin for power relay output

// Constants for timing
#define DATA_FREQ     5000  // 5 sec between data collections
#define WEB_TIMEOUT   1000  // ms before web client is booted off

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte           mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress      ip(59,167,158,82);
EthernetServer webserver(80);

//unsigned long curMillis;
unsigned long webTimeout;
unsigned long nextDataCheck;
EthernetClient client;      // global for better memory management

#if SD_EN == 0 // global vars for data storage if not using SD card
  #define DATA_SET 50    // number of recent data points to keep
  byte  dataIndex;
  short data[3][DATA_SET];
#endif
// Setup
// Return Type: none
// Description: performs once off setup operations on system startup
void setup()
{
  #if DEBUG
    Serial.begin(115200);   // start serial for Debugging
    Serial.println("\n\nQuoin Control System");
    Serial.println("--------------------");
    Serial.println("debugging enabled");
  #endif
  #if TIME_EN
    Wire.begin();             // start i2c
    //setDateTime();          // only use when RTC needs to be set
    #if DEBUG
      Serial.println("i2c interface started for RTC");
    #endif
  #endif
  #if WEB_EN
    Ethernet.begin(mac, ip);              // start ethernet
    W5100.setRetransmissionTime(0x07D0);  // wiznet ethernet chip timeout
    W5100.setRetransmissionCount(3);
    webserver.begin();        // start web server
    #if DEBUG
      Serial.println("ethernet + webserver started");
    #endif
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
    getDateTime();
  }
  // Respond to any  gooweb requests
  client = webserver.available();
  if (client)
  {
    webTimeout = millis() + WEB_TIMEOUT;
    String request = String();
    while (client.connected())
    {
      if (millis() > webTimeout)
      {
        #if DEBUG
          Serial.print(millis());
          Serial.println(" WEB TIMEOUT");
        #endif
        client.stop();
        delay(100);
        break;
      }
      if (client.available())
      {
        char c = client.read();
        // Process Request
        if (request.length() < 50)
        {
          request += c;
        }

        // Generate Response
        if (c == '\n')
        {
          #if DEBUG
            Serial.print("web request: ");
            Serial.print(request);
          #endif
          checkToggles(request);
          webprintHead();
          webprintHistory();
          webprintRelays();
          #if DEBUG_WEB
            webprintDebug(millis() - (webTimeout - WEB_TIMEOUT));
          #endif
          webprintEnd();
          // Give the web browser heaps of time to receive the data
          // 1ms is normally enough... but still...
          delay(1);
          client.stop();
        }
      }
    }
  }
}

//////////////////// IO OPERATIONS ////////////////////

// Check Data
// Return Type: none
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
  float current=(float)powerMax/1024*5/800*2000000;

  #if SD_EN
    int battery = map(analogRead(APIN_BATTERY), 0, 1024, 0, 1500);
    int solar   = map(analogRead(APIN_SOLAR),   0, 1024, 0, 1500);
  #else
    dataIndex = (dataIndex + 1) % DATA_SET;
    data[APIN_ACPOWER][dataIndex]=current/1.414;
    data[APIN_BATTERY][dataIndex] = map(analogRead(APIN_BATTERY), 0, 1024, 0, 1500);
    data[APIN_SOLAR][dataIndex]   = map(analogRead(APIN_SOLAR),   0, 1024, 0, 1500);
  #endif
}



// Toggle
// Return Type: NULL
// Description: Toggles the pin passed to it.
//              Used to toggle relay outputs.
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
