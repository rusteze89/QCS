/*
 * sd.ino
 * module for storing data on an SD card
 */
#if EN_SD

// make sure these includes are in the main file
// otherwise you may get compile errors
#include <SD.h>
#include <SPI.h>
#define PIN_SD  4         // pin for power relay output
#define SD_ERROR_START 1  // error code for when SD fails to start
#define SD_ERROR_WRITE 2  // error code for when SD fails to write
#define SD_ERROR_READ  3  // error code for when SD fails to read

byte sd_error_code;       // var for recording error code
#if EN_RTC
  byte sd_error_dt[7];    // var for storing date time of error
#endif

// Setup SD
// performs setup operations for SD card
void setupSD() {
  #if DEBUG_SER
    Serial.print("SD");
  #endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode(PIN_SD, OUTPUT);

  // see if the card is present and can be initialized:
  byte sdtry = 0;
  while (!SD.begin(PIN_SD) && sdtry < 3) {
    sdtry++;
    #if DEBUG_SER
      Serial.print(" Fail");
      Serial.print(sdtry);
    #endif
    delay(250);
    if (sdtry == 3)
    {
      sd_error_code = SD_ERROR_START;
      #if EN_RTC
        getDateTime(sd_error_dt);
      #endif
    }
  }
  #if DEBUG_SER
    if (sdtry == 0)
      Serial.println("   OK");
  #endif
}

// Read SD Analog
// Not functional at the moment
void readSDanalog() {
  #if DEBUG_SER
    Serial.print("read SD hist.csv...");
  #endif

  File sd = SD.open("hist.csv", FILE_READ);  // open analog log file

  if (sd) {
    #if DEBUG_SER
      Serial.println(" PASS");
    #endif
    // read values from file
  }
  else
  {
    sd_error_code = SD_ERROR_READ;
    #if EN_RTC
      getDateTime(sd_error_dt);
    #endif
    #if DEBUG_SER
      Serial.println(" FAIL");
    #endif
  }

  sd.close();                               // close analog log file
}

// Write SD Analog
// Writes parameters to SD card
void writeSDanalog() {
  if (sd_error_code == 0)
  {
    #if DEBUG_SER
      Serial.print("\nwrite SD hist.csv...");
    #endif
    // open the analog log file
//### maybe change to always open with flush() after write
    File sd = SD.open("hist.csv", FILE_WRITE);
    // if the file is available, write data
    if (sd) {                                 // if the file is open
      #if EN_RTC                              // print the time
        char dt[13];
        getDateTimeString(dt);
        sd.print(dt);
      #else
        sd.print(millis());
      #endif

      for (byte i=0; i<DATA_INPUTS; i++) {    // write analog data to sd card
        sd.print(',');
        sd.print(data[i][dataIndex]);
      }
      sd.println();                           // print a new line in the file
      sd.close();                             // close the file
      #if DEBUG_SER
        Serial.println(" PASS");
      #endif
    }
    else {                                    // otherwise the file isn't open
      sd_error_code = SD_ERROR_WRITE;         // so record the error
      #if EN_RTC
        getDateTime(sd_error_dt);
      #endif
      #if DEBUG_SER
        Serial.println(" FAIL");
      #endif
    }
  }
 #if DEBUG_SER
  else
  {
    Serial.print("SD Error ");
    Serial.println(sd_error_code);
  }
 #endif
}

#endif
