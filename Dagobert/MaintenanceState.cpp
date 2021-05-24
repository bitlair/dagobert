#include <LiquidCrystal_I2C.h>
#include "defs.h"
#include "FiniteStateMachine.h"
#include <Keypad.h>

extern LiquidCrystal_I2C lcd;
extern State unlockedState;
extern FSM fsm;
extern Keypad keypad;

void maintenance_update_external_devices() {
  lcd.setCursor(0, 0);
  lcd.print(digitalRead(AirAssistPin) ? "*" : " ");
  lcd.setCursor(0, 1);
  lcd.print(digitalRead(VentilationPin) ? "*" : " ");
  lcd.setCursor(0, 2);
  lcd.print(digitalRead(LaserEnablePin) && digitalRead(RuidaEnablePin) ? "*" : " ");
}

void maintenance_state_enter() {
  digitalWrite(IButtonGreenLedPin, HIGH);
  
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("* 1 air      ");
  lcd.setCursor(0,1);
  lcd.print("  2 vent     9 on ");
  lcd.setCursor(0,2);
  lcd.print("  3 laser    0 off  ");
  lcd.setCursor(0,3 );
  lcd.print("             # end");

  maintenance_update_external_devices();

  Serial.println("M");
}

void maintenance_state_leave() {
  Serial.println("L");
}

void maintenance_state_update() {
  char key = keypad.getKey();
  switch(key) {
    case '1': digitalWrite(AirAssistPin, !digitalRead(AirAssistPin)); break;
    case '2': digitalWrite(VentilationPin, !digitalRead(VentilationPin)); break;
    case '3': digitalWrite(LaserEnablePin, !digitalRead(LaserEnablePin));
              digitalWrite(RuidaEnablePin, !digitalRead(RuidaEnablePin)); break;
    case '9': digitalWrite(AirAssistPin, HIGH);
              digitalWrite(VentilationPin, HIGH);
              digitalWrite(LaserEnablePin, HIGH);
              digitalWrite(RuidaEnablePin, HIGH);
              break;
    case '0': digitalWrite(AirAssistPin, LOW);
              digitalWrite(VentilationPin, LOW);
              digitalWrite(LaserEnablePin, LOW);
              digitalWrite(RuidaEnablePin, LOW);
              break;
    case '#':
      fsm.transitionTo(unlockedState);
      break;
  }

  maintenance_update_external_devices();
}
