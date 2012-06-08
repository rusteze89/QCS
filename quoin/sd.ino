/*
 * sd.ino
 * module for storing data on an SD card
 */
#if SD_EN

// make sure these includes are in the main file
// otherwise you may get compile errors
#include <SD.h>
#include <SPI.h>
#define PIN_SD  4         // pin for power relay output
#define SD_ERROR_START 1  // error code for when SD fails to start
#define SD_ERROR_WRITE 2  // error code for when SD fails to write
#define SD_ERROR_READ  3  // error code for when SD fails to read

byte sd_error_code;       // var for recording error code
#if RTC_EN
  byte sd_error_dt[7];    // var for storing date time of error
#endif

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
    sd_error_code = SD_ERROR_START;
    #if RTC_EN
      getDateTime(sd_error_dt);
    #endif
    #if DEBUG
      Serial.println("FAIL");
    #endif
  }
#if DEBUG
  else
  {
    Serial.println("OK");
  }
#endif
}

// Read SD Analog
void readSDanalog() {
  #if DEBUG
    Serial.print("read SD ana.log...");
  #endif

  File sd = SD.open("ana.log", FILE_READ);  // open analog log file

  if (sd) {
    #if DEBUG
      Serial.println(" PASS");
    #endif
    // read values from file
  }
  else
  {
    sd_error_code = SD_ERROR_READ;
    #if RTC_EN
      getDateTime(sd_error_dt);
    #endif
    #if DEBUG
      Serial.println(" FAIL");
    #endif
  }

  sd.close();                               // close analog log file
}

// Write SD Analog
// Writes parameters to SD card
void writeSDanalog() {

  #if DEBUG
    Serial.print("write SD ana.log...");
  #endif
  // open the analog log file
  File sd = SD.open("ana.log", FILE_WRITE); // change to always open with flush() after write
  // if the file is available, write data
  if (sd) {
    // Start line with time
    #if RTC_EN
      char t[8];
      getTimeString(t);
      sd.print(t);
    #else
      sd.print(millis());
    #endif

    // print analog data
    for (byte i = 0; i < DATA_INPUTS; i++) {
      sd.print(',');
      sd.print(data[DATA_INPUTS][dataIndex]);
    }
    sd.println();
    // close the analog log file
    sd.close();
    #if DEBUG
      Serial.println(" PASS");
    #endif
  }
  else { // if the file isn't open
    sd_error_code = SD_ERROR_WRITE;
    #if RTC_EN
      getDateTime(sd_error_dt);
    #endif
    #if DEBUG
      Serial.println(" FAIL");
    #endif
  }
}

#endif
