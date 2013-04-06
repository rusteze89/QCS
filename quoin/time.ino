/*
 * time.ino
 * module for setting and getting the time from the real time clock
 * based off 
 */
#if EN_RTC

//#include <RTClib.h>
#include "Wire.h"

#define RTC_I2C_ADDR 0x68 // This is the I2C address
#define T_SEC   0
#define T_MIN   1
#define T_HOUR  2
#define T_DAYW  3
#define T_DAYM  4
#define T_MONTH 5
#define T_YEAR  6

// Setup Time
// performs setup operations for the Real Time Clock
void setupTime()
{
  Wire.begin();             // start i2c
//  byte t[] = {0,9,14,0,2,9,12}; // s,m,h,dw,dm,m,y
//  setDateTime(t);         // only use when RTC needs to be set
  #if DEBUG_SER
    Serial.println("RTC  OK");
  #endif
}

// Decimal to Binary Coded Decimal
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Binary Coded Decimal to Decimal
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}
 
// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers,
// Probably need to put in checks for valid numbers.
 
void setDateTime()                
{
  byte rtcDateTime[7];
  rtcDateTime[T_SEC]   = 10; // second  
  rtcDateTime[T_MIN]   = 15; // minute
  rtcDateTime[T_HOUR]  = 17; // hour
  rtcDateTime[T_DAYW]  = 0;  // day of the week
  rtcDateTime[T_DAYM]  = 27; // day of the month
  rtcDateTime[T_MONTH] = 5;  // month
  rtcDateTime[T_YEAR]  = 12; // year
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((byte)0);
  for (byte i = 0; i < 7; i++)
    Wire.write((byte)decToBcd(rtcDateTime[i]));     // 0 to bit 7 starts the clock
  Wire.endTransmission();
}

void setDateTime(byte dt[7])                
{
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((byte)0);
  for (byte i = 0; i < 7; i++)
    Wire.write((byte)decToBcd(dt[i]));     // 0 to bit 7 starts the clock
  Wire.endTransmission();
}
 
// Gets the date and time from the ds1307 and prints result
void getDateTime(byte rtcDateTime[7]) {
  // Reset the register pointer
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((byte)0);
  Wire.endTransmission();
 
  Wire.requestFrom(RTC_I2C_ADDR, 7);
 
  // A few of these need masks because certain bits are control bits
  byte i = 0;
  rtcDateTime[i++] = bcdToDec(Wire.read() & 0x7f);
  rtcDateTime[i++] = bcdToDec(Wire.read());
  rtcDateTime[i++] = bcdToDec(Wire.read() & 0x3f);  // Change for 12 hour am/pm
  while (i < 7)
    rtcDateTime[i++] = bcdToDec(Wire.read());
}

void getDateTimeString(char dt[13]) {
  byte rtcDateTime[7];
  getDateTime(rtcDateTime);
  getDateTimeString(dt, rtcDateTime);
  #if DEBUG_SER
    Serial.print("getDateTimeString(byte[7]) = ");
    Serial.print(dt);
  #endif
}

void getDateTimeString(char dt[13], byte rtcDateTime[7]) {
  // generate date string (YYMMDDhhmmss)
  dt[0]  = rtcDateTime[T_YEAR]  / 10 + 48;
  dt[1]  = rtcDateTime[T_YEAR]  % 10 + 48;
  dt[2]  = rtcDateTime[T_MONTH] / 10 + 48;
  dt[3]  = rtcDateTime[T_MONTH] % 10 + 48;
  dt[4]  = rtcDateTime[T_DAYM]  / 10 + 48;
  dt[5]  = rtcDateTime[T_DAYM]  % 10 + 48;
  dt[6]  = rtcDateTime[T_HOUR]  / 10 + 48;
  dt[7]  = rtcDateTime[T_HOUR]  % 10 + 48;
  dt[8]  = rtcDateTime[T_MIN]   / 10 + 48;
  dt[9]  = rtcDateTime[T_MIN]   % 10 + 48;
  dt[10] = rtcDateTime[T_SEC]   / 10 + 48;
  dt[11] = rtcDateTime[T_SEC]   % 10 + 48;
  dt[12] = '\0';
  #if DEBUG_SER
    Serial.print("datetime string: ");
    Serial.println(dt);
  #endif
}

#endif
