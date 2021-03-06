#include <Arduino.h>
#include <LiquidCrystal.h>
#include "defs.h"
#include "FiniteStateMachine.h"

extern LiquidCrystal lcd;
extern State lockedState;
extern FSM fsm;

void loading_state_enter() {
  lcd.clear();
  lcd.print("Bitlair Lasercutter");
  lcd.setCursor(0, 2);
  lcd.print("Waiting for Dagobert");

  digitalWrite(RuidaEnablePin, HIGH);
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
