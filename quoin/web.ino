/*
 * web.ino
 * module for printing data to the web interface
 */
#if WEB_EN

#define NUM_ANALOG 3  // number of analog logs to output

byte           mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress      ip(59,167,158,82);
EthernetServer webserver(80);
EthernetClient client;  // global for simpler memory management
unsigned long webTimeout;

void setupWeb()
{
  Ethernet.begin(mac, ip);              // start ethernet
  W5100.setRetransmissionTime(0x07D0);  // wiznet ethernet chip timeout
  W5100.setRetransmissionCount(3);
  webserver.begin();        // start web server
  #if DEBUG
    Serial.println("ethernet + webserver started");
  #endif
}

// Check Toggles
// checks received items from the web
void checkToggles(String request)
{
  if (request.indexOf("toggle=relay1") >= 0) {
    toggle(PIN_RELAY1);
  }

  if (request.indexOf("toggle=relay2") >= 0) {
    toggle(PIN_RELAY2);
  }
}

void webCheck()
{
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

// Web Print Head
// Prints the header of the http response
void webprintHead()
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/javascript");
  client.println();


  // callback({a:[],h0:[],h1:[],h2:[],r1:0,r2:1,m:547,p:133,run:32.1});

  client.print("callback({v:");
  client.print(VERSION);
}

// Web Print History
// sends the short term history data for each recorded input
void webprintHistory()
{
  #if SD_EN
    // print data if using SD
    // only prints datapoint in memory at this point
    for (byte i = 0; i < NUM_ANALOG; i++)
    {
      client.print(",h");
      client.print(i);
      client.print(":[");
      client.print(data[i][0]);
      client.print("]");
    }
  #else
    // print data using local variables if not using SD
    for (byte i = 0; i < NUM_ANALOG; i++)
    {
      client.print(",h");
      client.print(i);
      client.print(":[");
      byte j = (dataIndex + 1) % DATA_SET;
      while(j != dataIndex)
      {
        client.print(data[i][j]);
        client.print(",");
        j = (j + 1) % DATA_SET;
      }
      client.print(data[i][j]);
      client.print("]");
    }
  #endif
}

// Web Print Relays
// sends the state of the relay outputs
void webprintRelays()
{
  client.print(",r1:");
  client.print(digitalRead(PIN_RELAY1));
  client.print(",r2:");
  client.print(digitalRead(PIN_RELAY2));
}

// Web Print Debug
// sends debug information to the web front end
void webprintDebug(unsigned long time)
{
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
}

// Web Print End
// sends the end of the http response
void webprintEnd()
{
  // Fin
  client.print("});");
}

#endif