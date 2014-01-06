/*
 * quoin.ino
 * base code for quoin control system
 * 
 * Modules:
 * * web.ino    contains functions to process and send jquery callback
 *
 * To Do:
 * * fix ethernet hangup - usually under high load / different clients
 * * test time code & implement data timestamping
 * * implement SD card storage
 */

#include <avr/pgmspace.h>

#define VERSION       1.0   // Working over 3g with VPN (from router)

// Functionality Switches
#define EN_WEB        1     // enables web output functions

// Debugger enables
#define DEBUG_SER     0     // debug operation by dumping to serial
#define DEBUG_MEM     0     // will make system unstable since it fills ram
#define DEBUG_WEB     1     // sends debug information to the web front end

// General IO Pins used
#define APIN_ACPOWER  0     // analog pin for AC amps reading from inverter
#define APIN_BATTERY  1     // analog pin for battery voltage reading
#define APIN_SDA      5     // i2c SDA pin (analog 5)
#define APIN_SCL      6     // i2c SCL pin (analog 6)
#define PIN_RELAY1    2     // pin for power relay output
#define PIN_RELAY2    3     // pin for power relay output

// Constants for analog data
#define DATA_INPUTS   2     // number of inputs being collected
#define DATA_FREQ     1000  // 1s - time between data collections
#define DATA_AVG_SET  300   // 5m - #reads averaged into data point
#define DATA_SET      72    // 6h - #buffered data points
// DATA_SET will need to be reduced if debugging on serial
// due to the extra ram requirements of running the serial library

byte  dataIndex;
short dataLive[DATA_INPUTS];
short dAvgIndex;
short data[DATA_INPUTS][DATA_SET];
unsigned long dAvg[DATA_INPUTS];
unsigned long nextDataCheck;

// Setup
// performs once off setup operations on system startup
void setup()
{
  #if DEBUG_SER
    Serial.begin(115200);   // start serial for Debugging
    delay(1000);            // allow for serial to initalize
    Serial.println("\n\nQuoin Control System");
    delay(5000);
  #endif

  #if EN_WEB    // setup ethernet and web
    setupWeb();
  #endif

  // set outputs
  for (int i=0; i<8; i++)
    pinMode(i, OUTPUT);
  #if DEBUG_SER
    Serial.println("init complete");
  #endif

  nextDataCheck = millis() + millis() / 1000 * 1000 + 2000;
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
  #if EN_WEB
    webCheck();
  #endif

  #if EN_COSM
    cosmConCheck();
  #endif
}

// Check Data
// reads analog inputs into SD or global vars
void checkAnalogData()
{
  #if DEBUG_SER
    Serial.print("Check Analog ");
  #endif
  // AC live mesurement
  dataLive[APIN_ACPOWER] = 0;
  int val = 0;
  unsigned long tmeasure = millis() + 100;
  while (millis() < tmeasure)
  {
    val = analogRead(APIN_ACPOWER);             // read AC power (pin 0)
    if (val > dataLive[APIN_ACPOWER])
      dataLive[APIN_ACPOWER] = val;          // record the maximum sensor value
  }
  #if DEBUG_WEB
    dataLive[0] = val;
  #endif
  #if DATA_AVG_SET > 0                              // if taking average values
    dAvgIndex = (dAvgIndex + 1) % DATA_AVG_SET;     // increment avg index
    dAvg[APIN_ACPOWER] += dataLive[APIN_ACPOWER];// record data point
    dataLive[APIN_BATTERY] = analogRead(APIN_BATTERY);
    dAvg[APIN_BATTERY] += dataLive[APIN_BATTERY];
    if (dAvgIndex == DATA_AVG_SET - 1)
    {
      dataIndex = (dataIndex + 1) % DATA_SET;       // increment the data index
      for (byte i=0; i<DATA_INPUTS; i++)
      {                                             // go through each input
        data[i][dataIndex] = dAvg[i] / DATA_AVG_SET;// divide by the number of points
        dAvg[i] = 0;                                // reset average for next round
      }
    }
    #if DEBUG_SER
     else {
      Serial.print(dataIndex);
      Serial.print(".");
      Serial.println(dAvgIndex);
     }
    #endif
  #else                                             // not averaging data
    dataIndex = (dataIndex + 1) % DATA_SET;         // increment the data index
    data[APIN_ACPOWER][dataIndex] = dataLive[APIN_ACPOWER];// record the data point
    data[APIN_BATTERY][dataIndex] = analogRead(APIN_BATTERY);
    #if DEBUG_SER
      Serial.println(dataIndex);
    #endif
  #endif
}

// Toggle
// Description: Toggles the pin passed to it. Used to toggle relay outputs.
void toggle(byte pin) {
  #if DEBUG_SER
    Serial.print("Toggle Switch ");
    Serial.println(pin);
  #endif
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
short memoryTest() {
  short byteCounter = 0;
  byte  *byteArray;

  while ((byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL) {
    byteCounter++;
    free(byteArray);
  }

  free(byteArray);
  return byteCounter;
}
