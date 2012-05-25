/*
 * time.ino
 * module for setting and getting the time from the real time clock
 * based off 
 */

#include "Wire.h"
#define RTC_I2C_ADDR 0x68  // This is the I2C address
#define T_SEC 0
#define T_MIN 1
#define T_HOUR  2
#define T_DAYW 3
#define T_DAYM 4
#define T_MONTH 5
#define T_YEAR 6

byte dateTime[7];

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
 
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}
 
// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers, Probably need to put in checks for valid numbers.
 
void setDateTime()                
{
  dateTime[T_SEC]   = 10; // second  
  dateTime[T_MIN]   = 50; // minute
  dateTime[T_HOUR]  = 20; // hour
  dateTime[T_DAYW]  = 0;  // day of the week
  dateTime[T_DAYM]  = 23; // day of the month
  dateTime[T_MONTH] = 5;  // month
  dateTime[T_YEAR]  = 12; // year
  Wire.beginTransmission((byte)RTC_I2C_ADDR);
  Wire.write((byte)0);
  for (byte i = 0; i < 7; i++)
    Wire.write(decToBcd(dateTime[i]));     // 0 to bit 7 starts the clock
  Wire.endTransmission();
}
 
// Gets the date and time from the ds1307 and prints result
void getDateTime()
{
  // Reset the register pointer
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((byte)0);
  Wire.endTransmission();
 
  Wire.requestFrom(RTC_I2C_ADDR, 7);
 
  // A few of these need masks because certain bits are control bits
  byte i = 0;
  dateTime[i++] = bcdToDec(Wire.read() & 0x7f);
  dateTime[i++] = bcdToDec(Wire.read());
  dateTime[i++] = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  while (i < 7)
    dateTime[i++] = bcdToDec(Wire.read());

  Serial.print(dateTime[T_HOUR], DEC);
  Serial.print(":");
  Serial.print(dateTime[T_MIN], DEC);
  Serial.print(":");
  Serial.print(dateTime[T_SEC], DEC);
  Serial.print("  ");
  Serial.print(dateTime[T_DAYM], DEC);
  Serial.print("/");
  Serial.print(dateTime[T_MONTH], DEC);
  Serial.print("/");
  Serial.println(dateTime[T_YEAR], DEC);
}
