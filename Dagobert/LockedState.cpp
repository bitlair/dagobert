#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "OneWire.h"
#include "defs.h"
#include "FiniteStateMachine.h"

extern LiquidCrystal_I2C lcd;
extern UserData loggedInUser;
extern State unlockedState;
extern FSM fsm;
extern volatile long laserActiveTime;
extern long ventilationDisableTime;

#define ADDRSIZE 8

void printButtonAddress(uint8_t *addr) {
  char buf[3];
  
  for(int i = 0; i < ADDRSIZE; i++) {
    snprintf(buf, sizeof(buf), "%02x", addr[i]);
    Serial.print(buf);
  }
}

OneWire iButtonOneWire(IButtonReaderPin);
long lastAuthenticationTime;

void locked_state_enter() {
  lastAuthenticationTime = 0;
  
  lcd.clear();
  lcd.print("Bitlair Lasercutter");
  lcd.setCursor(0, 2);
  lcd.print("Bied iButton aan");

  digitalWrite(IButtonRedLedPin, HIGH);
  digitalWrite(IButtonGreenLedPin, LOW);

  digitalWrite(AirAssistPin, LOW);
  digitalWrite(LaserEnablePin, LOW);
  digitalWrite(RuidaEnablePin, HIGH);

  if ((millis() - laserActiveTime) > MINIMUM_VENTILATION_TIME) {
    digitalWrite(VentilationPin, LOW);
  } else {
    ventilationDisableTime = laserActiveTime + MINIMUM_VENTILATION_TIME;
  }
}

void locked_state_leave() {
  ventilationDisableTime = 0;
  
  digitalWrite(IButtonRedLedPin, LOW);
  digitalWrite(IButtonGreenLedPin, LOW);
}

void checkSerialForInstructions() {
  if (Serial.available()) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    
    /*
    F<reason>
    U<maintenance><cents><account>  
    */
    Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);

    if (buffer[0] == 'F') {
      lcd.setCursor(0, 2);
      lcd.print("                   ");
      lcd.setCursor(0, 2);
      lcd.printstr(buffer + 1);

      lastAuthenticationTime = millis();
    } else if (buffer[0] == 'U') {
      sscanf(buffer, "U%*c%d%s", &loggedInUser.centsPerMinute, loggedInUser.accountName);

      loggedInUser.maintenanceAllowed = buffer[1] == 'T';
      loggedInUser.costOfLastJob = 0;
      loggedInUser.totalCost = 0;
      
      fsm.transitionTo(unlockedState);
    }
  }
}

void locked_state_update() {
  uint8_t addr[ADDRSIZE];

  if (!lastAuthenticationTime) {
    iButtonOneWire.reset_search();
    
    if (iButtonOneWire.search(addr) && OneWire::crc8(addr, 7) == addr[7]) {
      lcd.setCursor(0, 2);
      lcd.print("Authenticating   ");

      lastAuthenticationTime = millis();

      digitalWrite(IButtonGreenLedPin, HIGH);

      Serial.print("A");
      printButtonAddress(addr);
      Serial.print("\n");
    }
  } else if ((millis() - lastAuthenticationTime) > AUTHENTICATION_TIMEOUT) {
    locked_state_enter();
  }

  checkSerialForInstructions();
}
