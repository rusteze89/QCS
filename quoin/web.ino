/*
 * web.ino
 * module for printing data to the web interface
 */
#if EN_WEB

#include <Ethernet.h>
#include <utility/w5100.h>

#define WEB_TIMEOUT   5000  // ms before web client is booted off

EthernetServer  webserver(80);
EthernetClient  client;  // global for simpler memory management
unsigned long   webTimeout;

// Setup Web
// performs setup operations for ethernet and web
void setupWeb() {
  #if DEBUG_SER
    Serial.print("WEB");
  #endif
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEF };// mac address
  byte ip[]  = { 192, 168, 88, 2 };     // ip address
  Ethernet.begin(mac, ip);              // start ethernet
  webserver.begin();                    // start web server
  #if DEBUG_SER
    Serial.println("  OK");
  #endif
}

// Web Check
// checks the incomming http GET 
void webCheck() {
  client = webserver.available();
  if (client) {
    webTimeout = millis() + WEB_TIMEOUT;
    char request[50];
    byte reqIndex = 0;
    while (client.connected()) {
      if (millis() > webTimeout) {
        #if DEBUG_SER
          Serial.print(millis());
          Serial.println(" WEB TIMEOUT");
        #endif
        client.stop();
        delay(100);
        break;
      }
      if (client.available()) {
        char c = client.read();
        // Process Request
        if (reqIndex < 49)
          request[reqIndex++] = c;

        // Generate Response
        if (c == '\n') {
          #if DEBUG_SER
            Serial.print("web request: ");
            Serial.print(request);
          #endif
          webCheckRequest(request);

          // print out the response
          webPrintHead();
          webPrintCallback();

          // Give the web browser heaps of time to receive the data
          // 1ms is normally enough... but still...
          delay(1);
          client.stop();
        }
      }
    }
  }
}

// Check SD
// checks if a particular file or list was requested
void webCheckRequest(char request[]) {
  if (strstr(request, "r=r1") != NULL)
    toggle(PIN_RELAY1);
  if (strstr(request, "r=r2") != NULL)
    toggle(PIN_RELAY2);
}

// Web Print Head
// prints the header of the http response
void webPrintHead() {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/javascript");
  client.println();
}

// Web Print Callback
// collects and prints callback response
void webPrintCallback() {
  //callback({dt:'23:59:59 31/12/2012',
  //  h0:[],h1:[],h2:[],r1:0,r2:1,m:547,p:133,run:32.1});
  client.print("callback({v:");             // print start of callback
  client.print(VERSION);                    // print code version number

  // print history
  #if EN_RTC                                // if the real time clock is on
    char datetimeString[13];
    getDateTimeString(datetimeString);      // get the time
    client.print(",dt:'");                  // and send it as a string
    byte i = 6;                             // the element where the time starts
    while (i < 12) {                        // the end of the time string
      client.print(datetimeString[i++]);
      client.print(datetimeString[i++]);
      if (i < 11) {                         // if it's not up to the secs
        client.print(":");                  // print time seperator
      }
    }
    client.print("'");                      // print end quotation
  #else
    client.print(",dt:'N/A'");              // print not available
  #endif
  for (byte i = 0; i < DATA_INPUTS; i++) {  // print of data
    client.print(",h");
    client.print(i);
    client.print(":[");
    #if DATA_SET > 1                        // if there's a data set
    byte j = (dataIndex + 1) % DATA_SET;    // start at the oldest data point
      while(j != dataIndex) {               // and print all until reaching
        client.print(data[i][j]);           // the newest data point
        client.print(",");                  // with commas to separate values
        j = (j + 1) % DATA_SET;
      }
      client.print(data[i][j]);
    #else                                   // if no data set
      client.print(data[i][dataIndex]);     // then no need to loop
    #endif                                  // so I added a small optimisation
    client.print("]");
  }

  client.print(",r1:");                     // print relay 1's output state
  client.print(digitalRead(PIN_RELAY1));
  client.print(",r2:");                     // print relay 2's output state
  client.print(digitalRead(PIN_RELAY2));

  #if DEBUG_WEB                             // print debug info if enabled
    client.print(",debug:'PageGen:");       // print debug
    client.print(millis() - (webTimeout - WEB_TIMEOUT));//print page gen time
    client.print("ms Runtime:");
    webTimeout = millis();
    client.print(webTimeout / 3600000);     // print runtime hours
    client.print("h");
    client.print(webTimeout / 60000 % 60);  // print runtime mins
    client.print("m");
    #if EN_SD                               // print SD error code
      if (sd_error_code)                    // if one exists
      {
        client.print(" SD:");
        client.print(sd_error_code);
        #if EN_RTC
          client.print(" ");
          getDateTimeString(datetimeString, sd_error_dt);
          client.print(datetimeString);
        #endif
      }
    #endif
    #if DEBUG_MEM
      client.print(" FreeMem:");
      client.print(memoryTest());           // print free memory
      client.print("b");
    #endif
    client.print("'");                      // print end of debug string
  #endif
  client.println("});");                    // print end of callback
}

#endif
