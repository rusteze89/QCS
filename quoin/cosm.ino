/*
 Cosm / Pachube sensor client
 
 This sketch connects an analog sensor to Pachube (http://www.pachube.com)
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.
 
 This example has been updated to use version 2.0 of the Pachube.com API. 
 To make it work, create a feed with a datastream, and give it the ID
 sensor1. Or change the code below to match your feed.
 
 
 Circuit:
 * Analog sensor attached to analog in 0
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 15 March 2010
 updated 16 Mar 2012
 by Tom Igoe with input from Usman Haque and Joe Saavedra
 
http://arduino.cc/en/Tutorial/PachubeClient
 This code is in the public domain.
 
 */
#if EN_COSM

#include <SPI.h>
#include <Ethernet.h>
// #include <utility/w5100.h>

#define APIKEY         "NP02dDi2zPZz3YLTGHjMaBQuNDmSAKxua3BkaVQ0Y3RaVT0g" // your cosm api key
#define FEEDID         63821 // your feed ID
#define USERAGENT      "Quoin Control (63821)" // user agent is the project name

// initialize the library instance:
EthernetClient client;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(216,52,233,121);      // numeric IP for api.cosm.com
//char server[] = "api.cosm.com";   // name address for cosm API

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to Pachube.com

void setupCosm() {
  #if DEBUG_SER
    Serial.print("COSM");
  #endif
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEF };// mac address
  IPAddress ip(59,167,158,82);
  Serial.print(" MAC");

  // start the Ethernet connection:
  byte ethtry = 0;
  while (!Ethernet.begin(mac) && ethtry < 3) {
    ethtry++;
    #if DEBUG_SER
      Serial.print(" Fail");
      Serial.print(ethtry);
    #endif
    delay(1000);
  }

  #if DEBUG_SER
    if (ethtry == 0)
      Serial.println(" OK");
  #endif
}

void cosmConCheck() {
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    #if DEBUG_SER
      Serial.println();
      Serial.println("disconnecting.");
    #endif
    client.stop();

    if (cosmFlag)
      cosmSend();
  }

  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void cosmSend() {
  #if DEBUG_SER
    Serial.print("cosm.com");
  #endif
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    #if DEBUG_SER
      Serial.println(" connected");
    #endif
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.cosm.com");
    client.print("X-ApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");

    // calculate the length of the sensor reading in bytes:
    // 3 bytes for "a0," + number of digits of the data:
    // int thisLength = 8 + cosmGetLength(data[0][dataIndex]);
    int thisLength = 0;
    for (byte i = 0; i < DATA_INPUTS; i++) {
      // 3 bytes for "a0," + number of digits of the data:
      thisLength += 3 + cosmGetLength(data[i][dataIndex]);
    }
    client.println(thisLength);

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    // client.print("ACpower,");
    // client.println(data[0][dataIndex]);

    for (byte i = 0; i < DATA_INPUTS; i++) {
      client.print("a");
      client.print(i);
      client.print(",");
      client.println(data[i][dataIndex]);
    }
    cosmFlag = 0;
  }
  else {
    cosmFlag++;
    #if DEBUG_SER
      Serial.print(" fail ");
      Serial.println(cosmFlag);
    #endif
  }

  // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}


// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:

int cosmGetLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}

#endif