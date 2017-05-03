#include "defs.h"
#include <OneWire.h>

OneWire temperatureSensor(TemperatureSensorPin);
byte temperatureAddr[8];

int temperatureState;
long lastTemperatureTime;

void setupTemperatureSensor() {
  #ifdef DEBUG
  Serial.println("DChecking temperature sensor");
#endif
  temperatureSensor.reset_search();
  if (temperatureSensor.search(temperatureAddr) && OneWire::crc8(temperatureAddr, 7) == temperatureAddr[7]) {
#ifdef DEBUG    
    Serial.println("DFound temperature sensor");
#endif
  }
}

float convertToCelcius(byte *data, int type_s) {
  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) 
  {
    // 9 bit resolution default
    raw = raw << 3;
    if (data[7] == 0x10) 
    {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } 
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00)
    {
      // 9 bit resolution, 93.75 ms
      raw = raw << 3;
    }
    else if (cfg == 0x20) 
    {
      // 10 bit res, 187.5 ms
      raw = raw << 2;
    }
    else if (cfg == 0x40) 
    {
      // 11 bit res, 375 ms
      raw = raw << 1;
    }
    // default is 12 bit resolution, 750 ms conversion time
  }

  return (float) raw / 16.0;
}

void updateTemperatureReading() {
  if ((temperatureState == 0) && ((millis() - lastTemperatureTime) > TEMPERATURE_SENSOR_INTERVAL)) {
    temperatureSensor.reset();
    temperatureSensor.select(temperatureAddr);
    temperatureSensor.write(0x44, 1);
    
    lastTemperatureTime = millis();
    temperatureState = 1;
  }

  if ((temperatureState == 1) && ((millis() - lastTemperatureTime) > 1000)) {
    temperatureSensor.reset();
    temperatureSensor.select(temperatureAddr);
    temperatureSensor.write(0xBE);

    byte data[12];
    for(int i = 0; i < 9; i++) {
      data[i] = temperatureSensor.read();
    }

    if (OneWire::crc8(data, 8) == data[8]) {
      float temperature = convertToCelcius(data, (temperatureAddr[0] == 0x10));

      Serial.print("T");
      Serial.println(temperature);
    }

    temperatureState = 0;
  }
}


