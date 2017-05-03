#include "defs.h"

#define TotalPulsePeriods 5
int pulsePeriods[TotalPulsePeriods];
long lastPulse;
float waterFlowLitersPerMinute;
long lastWaterFlowMeasurement;

void waterPulseCounter() 
{
  if (lastPulse != 0) {
      long diff = millis() - lastPulse;
      memmove(pulsePeriods + 1, pulsePeriods, sizeof(pulsePeriods) - sizeof(int));
      pulsePeriods[0] = diff;
  }

  lastPulse = millis();
}

void updateWaterFlow() 
{
  long totalDifference = 0;
  int totalPeriods = 0;
  for(int i = 0; i < TotalPulsePeriods; i++) {
    if (pulsePeriods[i] != 0) {
      totalPeriods++;
      totalDifference += pulsePeriods[i];
    }
  }

  if (totalPeriods == 0) return;

  int averageDifference = totalDifference / totalPeriods;
  float pulsesPerMinute = 60000 / (float)averageDifference;

  // YF-G1 has 4.8 pulses per liter
  waterFlowLitersPerMinute = pulsesPerMinute / 4.8;

  if ((millis() - lastWaterFlowMeasurement) > WATERFLOW_SENSOR_INTERVAL) {
    Serial.print("W");
    Serial.println(waterFlowLitersPerMinute);
    lastWaterFlowMeasurement = millis();
  }
}

