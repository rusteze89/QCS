/*
 * io.ino
 * module for collecting data and setting outputs
 */

#define APIN_ACPOWER 0      // analog pin for AC amps reading off inverter
#define APIN_BATTERY 1      // analog pin for battery voltage reading
#define APIN_SOLAR   2      // analog pin for solar voltage reading
#define PIN_RELAY1   2      // pin for power relay output
#define PIN_RELAY2   3      // pin for power relay output

void checkData()
{
  dataIndex = (dataIndex + 1) % DATA_SET;

// AC current mesurement
  int powerMax = 0;
  int val = 0;
  currentTime = millis();
  unsigned long tmeasure = currentTime + 100;
  while (millis() < tmeasure)
  {
    val = analogRead(APIN_ACPOWER);

    // record the maximum sensor value
    if (val > powerMax)
      powerMax = val;
  }
  float current=(float)powerMax/1024*5/800*2000000;
  data[APIN_ACPOWER][dataIndex]=current/1.414;

// Battery voltage measurement
  data[APIN_BATTERY][dataIndex] = map(analogRead(APIN_BATTERY), 0, 1024, 0, 1500);

// Solar voltage measurement
  data[APIN_SOLAR][dataIndex]   = map(analogRead(APIN_SOLAR),   0, 1024, 0, 1500);
}



// Toggle.
// Toggles the pin passed to it. Used to toggle relay outputs.
void toggle(byte pin)
{
  if (digitalRead(pin)) {
    digitalWrite(pin, LOW);
  } 
  else {
    digitalWrite(pin, HIGH);
  }
}
