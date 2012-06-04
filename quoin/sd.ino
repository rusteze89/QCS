/*
 * sd.ino
 * module for storing data on an SD card
 */
#if SD_EN

// make sure these includes are in the main file
// otherwise you may get compile errors
#include <SdFat.h>
#define PIN_SD  4   // pin for power relay output



// Setup SD
// performs setup operations for SD card
void setupSD() {
  #if DEBUG
    Serial.print("SD  ");
  #endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode(PIN_SD, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(PIN_SD)) {
    #if DEBUG
      Serial.println("FAIL");
    #endif
  }
  #if DEBUG
    Serial.println("OK");
  #endif
}

// Write SD
// Writes parameters to SD card
void writeSDanalog() {
  #if DEBUG
    Serial.println("write SD alog.txt");
  #endif
  // open the analog log file
  File sd = SD.open("alog.txt", FILE_WRITE);
  // if the file is available, write data
  if (sd) {
    // Start line with time
    #if TIME_EN
      char t[8];
      getTimeString(t);
      sd.print(t);
    #else
      sd.print(millis());
    #endif

    // print analog data
    for (byte i = 0; i < DATA_INPUTS; i++) {
      sd.print(',');
      sd.print(data[DATA_INPUTS][1]);
    }
    sd.println();
    // close the analog log file
    sd.close();
  }
  #if DEBUG
  else { // if the file isn't open
      Serial.print("error opening file");
  }
  #endif
}

#endif
