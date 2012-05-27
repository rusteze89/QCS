/*
 * sd.ino
 * module for storing data on an SD card
 */

// make sure these includes are in the main file
// otherwise you may get compile errors
#include <SD.h>

#define PIN_SD  4   // pin for power relay output

byte setupSD()
{
  #if DEBUG
    Serial.print("Initializing SD card...");
  #endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode(PIN_SD, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(PIN_SD)) {
    #if DEBUG
      Serial.println("Card failed, or not present");
    #endif
    return 1;
  }
  #if DEBUG
    Serial.println("card initialized.");
  #endif
  return 0;
}

// Open SD
// open the file
// pre-arduino 1.0 one file can be open at a time
File openSD(char filename[]) {
  #if DEBUG
    Serial.println("openSD");
  #endif
  File dataFile = SD.open(filename, FILE_WRITE);
  return dataFile;
}

void closeSD(File toClose) {
  #if DEBUG
    Serial.println("Closing File");
  #endif
  toClose.close();
}

// Toggle.
// Toggles the pin passed to it. Used to toggle relay outputs.
void printSD(File sd, byte dt[], char str[], byte len) {
  #if DEBUG
    Serial.println("printSD");
  #endif
  // if the file is available, write data
  if (sd) {
    #if DEBUG
      Serial.println("Writing Data");
    #endif
    sd.print(dt[3] % 10 + dt[3] / 10);
    sd.print(dt[2] + dt[1]);
    sd.println(str);
    // print to serial for debugging
    Serial.println(str);
  }
  else { // if the file isn't open
    Serial.print("error opening file");
  }
}
