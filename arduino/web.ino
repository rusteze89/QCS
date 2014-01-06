/*
 * web.ino
 * module for printing data to the web interface
 */
#if EN_WEB

#include <Ethernet.h>
#include <SPI.h>
#include <utility/w5100.h>

#define WEB_TIMEOUT   5000      // ms before web client is booted off

EthernetServer  webserver(80);
EthernetClient  client;         // global for simpler memory management
unsigned long   webTimeout;     // keep track of how long the webserver
byte            dataIndexLast;  // the dataIndex value when data was last requested
byte            webReqData;     // set if data was requested
byte            webReqLive;     // set if live data was requested
byte            webReqDebug;    // set if debug data requested

// Setup Web
// performs setup operations for ethernet and web
void setupWeb() {
  #if DEBUG_SER
    Serial.print("WEB");
  #endif
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEF };// mac address
  byte ip[]  = { 192,168,88,2 };        // ip address
  Ethernet.begin(mac,ip);               // start ethernet
  delay(1000);                          // allow for ethernet chip to initialize
  webserver.begin();                    // start web server
  dataIndexLast = 0;
  #if DEBUG_WEB
    webReqDebug = 0;
  #endif
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
        if (reqIndex < 48)
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
  if (strstr(request, "*") != NULL) {
    webReqData = 1;
    webReqLive = 1;
    webReqDebug = 1;
  }
  if (strstr(request, "data") != NULL)
    webReqData = 1;
  if (strstr(request, "live") != NULL)
    webReqLive = 1;
  if (strstr(request, "debug") != NULL)
    webReqDebug = 1;
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
  // print JSON starting brace
  client.println("{");

  // Collected and averaged Analog Data
  if (webReqData) {
    // print analog data JSON
    client.print("  \"analog_data\":[\n\t[");
    for (byte i = 0; i < DATA_INPUTS; i++) {// print off data
      byte j = dataIndexLast;               // start at the last data point sent
      while(j != dataIndex) {               // and print all until reaching
        client.print(data[i][j]);           // the newest data point
        client.print(",");                  // with commas to separate values
        j = (j + 1) % DATA_SET;             // increment around the dataset
      }
      client.print(data[i][j]);             // print newest data point for that input
      if (i < DATA_INPUTS -1)
        client.print("],\n\t[");            // end input and prepare for the next input
    }
    client.print("]\n  ]");                 // end of analog data
    dataIndexLast = dataIndex;              // remember the last point sent
  }

  // Live Data Values
  if (webReqLive) {
    if (webReqData)
      client.println(",");
    // print start of live data object
    client.println("  \"live_data\":{");
    // print live ac 
    client.print("\t\"AC\":");
    client.print(dataLive[APIN_ACPOWER]);   // live value of the AC Power pin
    client.println(",");
    // print live battery voltage
    client.print("\t\"Battery\":");
    client.print(dataLive[APIN_BATTERY]);   // live value of the battery pin
    client.println(",");
    // print relay outputs
    client.print("\t\"relay\":[");
    client.print(digitalRead(PIN_RELAY1));  // state of relay 1
    client.print(",");
    client.print(digitalRead(PIN_RELAY2));  // state of relay 2
    client.println("]");
    // print end of live data object
    client.print("  }");
  }

  #if DEBUG_WEB                             // print debug info if enabled
  if (webReqDebug) {                        // and it's been requested
    if (webReqData || webReqLive)
      client.println(",");
    client.println("  \"debug\":{");
    // print page generation time
    client.print("\t\"PageGen\":\"");       // print debug
    client.print(millis() - (webTimeout - WEB_TIMEOUT));//print page gen time
    client.println("ms\",");
    // print runtime
    webTimeout = millis() / 1000;
    client.print("\t\"Runtime\":\"");
    client.print(webTimeout / 86400);       // print runtime days
    client.print("d");
    client.print(webTimeout / 3600 % 24);   // print runtime hours
    client.print("h");
    client.print(webTimeout / 60 % 60);     // print runtime mins
    client.println("m\",");
    // print number of inputs
    client.print("\t\"Inputs\":");
    client.print(DATA_INPUTS);              // print #inputs being measured
    client.println(",");
    // print data collection frequency
    client.print("\t\"DataFreq\":");
    client.print(DATA_FREQ);                // time between data collections
    client.println(",");
    // print data index
    client.print("\t\"DataIndex\":");
    client.print(dataIndex);
    client.println(",");
    // print data average set
    client.print("\t\"DataAvgSet\":");
    client.print(DATA_AVG_SET);             // #reads averaged into data point
    client.println(",");
    // print size of data set
    client.print("\t\"DataSet\":");
    client.print(DATA_SET);                 // # data points in chart
    #if DEBUG_MEM
      client.println(",");
      // print free memory
      client.print("\t\"FreeMem\":\"");
      client.print(memoryTest());
      client.print("b\"");
    #endif
    // print end of debug object
    client.print("\n  }");
  }
  #else
    else if (webReqDebug) {
      webReqDebug = 0;
      if (webReqData || webReqLive)
        client.println(",");
      client.println("  \"debug\":\"information not available\"");
    }
  #endif
  // print JSON closing brace
  client.println("\n}");

  webReqData = 0;
  webReqLive = 0;
  webReqDebug = 0;
}

#endif
