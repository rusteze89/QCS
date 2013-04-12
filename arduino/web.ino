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

// Setup Web
// performs setup operations for ethernet and web
void setupWeb() {
  #if DEBUG_SER
    Serial.print("WEB");
  #endif
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEF };// mac address
  byte ip[]  = { 192,168,88,2 };        // ip address
  Ethernet.begin(mac,ip);               // start ethernet
  webserver.begin();                    // start web server
  #if DEBUG_SER
    Serial.println("  OK");
  #endif
  dataIndexLast = 0;
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
  // print analog data JSON
  client.print("{\"arduino_analog_data\":[[");
  for (byte i = 0; i < DATA_INPUTS; i++) {  // print off data
    byte j = dataIndexLast;                 // start at the last data point sent
    while(j != dataIndex) {                 // and print all until reaching
      client.print(data[i][j]);             // the newest data point
      client.print(",");                    // with commas to separate values
      j = (j + 1) % DATA_SET;               // increment around the dataset
    }
    client.print(data[i][j]);               // print newest data point for that input
    if (i < DATA_INPUTS -1)
      client.print("],[");                  // end input and prepare for the next input
  }
  client.print("]]}\n");                    // end of analog data
  dataIndexLast = dataIndex;                // remember the last point sent

  // print current data
  client.print("{\"current_data\":{");
  // print current ac 
  client.print("\n {\"AC\":");
  client.print(dataCurrent[APIN_ACPOWER]);  // current value of the AC Power pin
  client.print("},\n");
  // print current battery voltage
  client.print(" {\"Battery\":");
  client.print(dataCurrent[APIN_BATTERY]);  // current value of the battery pin
  client.print("},\n");
  // print relay outputs
  client.print(" {\"relay\":[");
  client.print(digitalRead(PIN_RELAY1));    // state of relay 1
  client.print(",");
  client.print(digitalRead(PIN_RELAY2));    // state of relay 2
  client.print("]}\n");
  client.print("}}\n");

  #if DEBUG_WEB                             // print debug info if enabled
    client.print("{\"debug\":{");
    // print page generation time
    client.print("\n {\"PageGen\":\"");        // print debug
    client.print(millis() - (webTimeout - WEB_TIMEOUT));//print page gen time
    client.print("ms\"");
    // print runtime
    webTimeout = millis() / 1000;
    client.print("},\n {\"Runtime\":\"");
    client.print(webTimeout / 86400);       // print runtime days
    client.print("d");
    client.print(webTimeout / 3600 % 24);   // print runtime hours
    client.print("h");
    client.print(webTimeout / 60 % 60);     // print runtime mins
    client.print("m\"");
    // print number of inputs
    client.print("},\n {\"Inputs\":");
    client.print(DATA_INPUTS);              // print #inputs being measured
    // print data collection frequency
    client.print("},\n {\"DataFreq\":");
    client.print(DATA_FREQ);                // time between data collections
    // print data index
    client.print("},\n {\"DataIndex\":");
    client.print(dataIndex);
    // print data average set
    client.print("},\n {\"DataAvgSet\":");
    client.print(DATA_AVG_SET);             // #reads averaged into data point
    // print size of data set
    client.print("},\n {\"DataSet\":");
    client.print(DATA_SET);                 // # data points in chart
    #if DEBUG_MEM
      client.print("},\n {\"FreeMem\":\"");
      client.print(memoryTest());           // print free memory
      client.print("b\"");
    #endif
    client.print("}\n}}\n");                 // print end of debug string
  #endif
}

#endif
