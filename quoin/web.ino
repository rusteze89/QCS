/*
 * web.ino
 * module for printing data to the web interface
 */

#define NUM_ANALOG 3  // number of analog logs to output

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
  #else
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
    client.print(",debug:'Page Gen:");
    client.print(time);
    client.print(" Runtime:");
    client.print((float)millis()/60000);
    #if DEBUG_MEM
      client.print(" Free Mem:");
      client.print(memoryTest());
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
