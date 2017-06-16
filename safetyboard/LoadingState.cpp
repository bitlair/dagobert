#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "OneWire.h"
#include "defs.h"
#include "FiniteStateMachine.h"

extern LiquidCrystal_I2C lcd;
extern State lockedState;
extern FSM fsm;

void loading_state_enter() {
  lcd.clear();
  lcd.print("Bitlair Lasercutter");

  digitalWrite(LightsPin, HIGH);
}

void loading_state_leave() {
}


void loading_state_update() {
  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
    
  if (Serial.available()) {
    Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    if (buffer[0] == 'R') {
      fsm.transitionTo(lockedState);
    }
  }  
}

