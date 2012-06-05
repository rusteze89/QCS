/*
 * web.ino
 * module for printing data to the web interface
 */
#if WEB_EN

#include <Ethernet.h>
#include <utility/w5100.h>

#define WEB_TIMEOUT   5000  // ms before web client is booted off

EthernetServer  webserver(80);
EthernetClient  client;  // global for simpler memory management
unsigned long   webTimeout;

// Setup Web
// performs setup operations for ethernet and web
void setupWeb() {
  byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEF };// mac address
  byte ip[]  = { 59,167,158,82 };       // ip address
  Ethernet.begin(mac, ip);              // start ethernet
  webserver.begin();                    // start web server
  #if DEBUG
    Serial.println("WEB OK");
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
        #if DEBUG
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
        if (reqIndex < 50) {
          request[reqIndex++] = c;
        }

        // Generate Response
        if (c == '\n') {
          #if DEBUG
            Serial.print("web request: ");
            Serial.print(request);
          #endif
          webPrintHead();
          switch (webCheckRequest(request)) {
            case  0: break; // no callback
            #if SD_EN
              case  1: webPrintFileList(LS_SIZE); break;
            #endif
            default: webPrintCallback();
          }
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
byte webCheckRequest(char request[]) {
  if (strstr(request, "r=r1") >= 0) {
    toggle(PIN_RELAY1);
    return 1; // 1 to indicate output 1
  }
  if (strstr(request, "r=r2") >= 0) {
    toggle(PIN_RELAY2);
    return 2; // 2 to indicate output 2
  }
  // if (strstr(request, "alog.txt") >=0) {
  //     client.println("Printing alog.txt");
  //     webPrintFile("alog.txt");
  //   return 0; // 0 for no callback
  // }
  return 255;
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
  client.print("callback({v:");
  client.print(VERSION);

  // print history
  char timeString[9];
  getTimeString(timeString);
  client.print(",dt:'");
  client.print(timeString);
  client.print("'");
  for (byte i = 0; i < DATA_INPUTS; i++) {
    client.print(",h");
    client.print(i);
    client.print(":[");
    #if DATA_SET > 1
    byte j = (dataIndex + 1) % DATA_SET;
      while(j != dataIndex) {
        client.print(data[i][j]);
        client.print(",");
        j = (j + 1) % DATA_SET;
      }
      client.print(data[i][j]);
    #else
      client.print(data[i][dataIndex]);
    #endif
    client.print("]");
  }

  // print relay states
  client.print(",r1:");
  client.print(digitalRead(PIN_RELAY1));
  client.print(",r2:");
  client.print(digitalRead(PIN_RELAY2));

  // print debug info if enabled
  #if DEBUG_WEB
    client.print(",debug:'Page:");
    client.print(time);
    client.print("ms Runtime:");
    client.print((float)millis()/60000);
    client.print("mins");
    #if DEBUG_MEM
      client.print(" Free Mem:");
      client.print(memoryTest());
      client.print("bytes free");
    #endif
    client.print("'");
  #endif
  client.print("});");
}

// Web Print File
// dumps all the data from a file on the SD card to web output
void webPrintFile(char filename[]) {
  #if SD_EN
    File dataFile = SD.open(filename);
    if (dataFile) {
      int x = 0;
      while (dataFile.available() && x < 30000) {
        client.write(dataFile.read());
        x++;
        #if DEBUG
        if (x % 1000 == 0) {
          Serial.print(x);
          Serial.write(' ');
          Serial.println(memoryTest());
        }
        #endif
      }
      dataFile.close();
    }
    else
      client.print("Could not open file");
  #else
    client.print("SD card not enabled");
  #endif
}

void webPrintFileList(uint8_t flags) {
  client.println("<h1>File List</h1><br/>");
 #if SD_EN
  // This code is just copied from SdFile.cpp in the SDFat library
  // and tweaked to print to the client output in html!
  dir_t p;
  
  root.rewind();
  client.println("<ul>");
  while (root.readDir(p) > 0) {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break;

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;

    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) continue;

    // print any indent spaces
    client.print("<li><a href=\"");
    for (uint8_t i = 0; i < 11; i++) {
      if (p.name[i] == ' ') continue;
      if (i == 8) {
        client.print('.');
      }
      client.print((char)p.name[i]);
    }
    client.print("\">");
    
    // print file name with possible blank fill
    for (uint8_t i = 0; i < 11; i++) {
      if (p.name[i] == ' ') continue;
      if (i == 8) {
        client.print('.');
      }
      client.print((char)p.name[i]);
    }
    
    client.print("</a>");
    
    if (DIR_IS_SUBDIR(&p)) {
      client.print('/');
    }

    // print modify date/time if requested
    if (flags & LS_DATE) {
       root.printFatDate(p.lastWriteDate);
       client.print(' ');
       root.printFatTime(p.lastWriteTime);
    }
    // print size if requested
    if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) {
      client.print(' ');
      client.print(p.fileSize);
    }
    client.println("</li>");
  }
  client.println("</ul>");
 #else
  client.println("SD not enabled");
 #endif
}

#endif