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

  client.print("callback({v:0.3");
}

// Web Print Instant
// sends the instantaneous data
void webprintInstant()
{
  client.print(",a:[");
  client.print(data[0][dataIndex]); // this should be the AC amps data
  for (byte i = 1; i < NUM_ANALOG; i++)
  {
    client.print(",");
    client.print(map(analogRead(i), 0, 1024, 0, 1500));
  }
  client.print("]");
}

// Web Print History
// sends the short term history data for each recorded input
void webprintHistory()
{
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

// Web Print Page Gen
// sends the page generation time
// (up to the point where this function is called)
void webprintPageGen(unsigned long time)
{
  client.print(",p:");
  client.print(time);
  client.print(",run:");
  client.print((float)millis()/60000);
}

// Web Print Memory
// sends the free memory of the arduino
void webprintMemory(short mem)
{
  client.print(",m:");
  client.print(mem);
}

// Web Print End
// sends the end of the http response
void webprintEnd()
{
  // Fin
  client.print("});");
}
