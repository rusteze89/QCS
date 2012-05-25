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

#include <SPI.h>
#include <Ethernet.h>
#include <RTClib.h>
#include "Wire.h"
#include <avr/pgmspace.h>
#include <utility/w5100.h>

#define DEBUG       1
#define DATA_SET    50   // number of recent data points to keep
#define DATA_FREQ   5000  // 5 sec between data collections
#define WEB_TIMEOUT 1000  // ms before web client is booted off


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte           mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress      ip(59,167,158,82);
EthernetServer webserver(80);

short data[3][DATA_SET];
byte  dataIndex;
unsigned long currentTime;
unsigned long webTimeout;
unsigned long nextDataCheck;
EthernetClient client;      // global for better memory management

void setup()
{
#if DEBUG
  Serial.begin(115200);     // start serial for Debugging
  Serial.println("\n\nQuoin Control System");
  Serial.println("--------------------");
  Serial.println("serial debugging");
#endif
//  setDateTime();            // set the RTC (until i put a battery in)
  // start ethernet and tell wiznet ethernet chip
  // to timeout if it can't connect for response
  Ethernet.begin(mac, ip);
  W5100.setRetransmissionTime(0x07D0);
  W5100.setRetransmissionCount(3);

  webserver.begin();        // start web server
  // set outputs
  for (int i=0; i<8; i++)
    pinMode(i, OUTPUT);
#if DEBUG
  Serial.println("initialization complete");
#endif
}

void loop()
{
  // Check if it's time to update data
  currentTime = millis();
  if (currentTime > nextDataCheck)
  {
    nextDataCheck = currentTime + DATA_FREQ;
    checkData();
//    getDateTime();
  }
  // Respond to any  gooweb requests
  client = webserver.available();
  if (client)
  {
    currentTime = millis();
    webTimeout = currentTime + WEB_TIMEOUT;
    String request = String();
    while (client.connected())
    {
      if (currentTime > webTimeout)
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
        if (request.length() < 100)
        {
          request += c;
        }

        // Generate Response
        if (c == '\n')
        {
          checkToggles(request);
          webprintHead();
          webprintInstant();
          webprintHistory();
          webprintRelays();
          currentTime = millis();
          webprintPageGen(currentTime - (webTimeout - WEB_TIMEOUT));
          // webprintMemory(memoryTest());
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
