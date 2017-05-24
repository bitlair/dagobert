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
  lcd.print(digitalRead(WaterCoolerPin) ? "*" : " ");
  lcd.setCursor(0, 2);
  lcd.print(digitalRead(VentilationPin) ? "*" : " ");
  lcd.setCursor(0, 3);
  lcd.print(digitalRead(LaserEnablePin) && digitalRead(LaosEnablePin) ? "*" : " ");
  lcd.setCursor(11, 0);
  lcd.print(digitalRead(TestfireEnablePin) && digitalRead(PotmeterEnable) ? "*" : " ");
  
}

void maintenance_state_enter() {
  digitalWrite(IButtonGreenLedPin, HIGH);
  
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("* 1 air      5 test");
  lcd.setCursor(0,1);
  lcd.print("* 2 water    9 on ");
  lcd.setCursor(0,2);
  lcd.print("  3 vent     0 off  ");
  lcd.setCursor(0,3 );
  lcd.print("  4 laser    # end");

  maintenance_update_external_devices();

  Serial.println("M");
}

void maintenance_state_leave() {
  Serial.println("L");
  
  digitalWrite(TestfireEnablePin, LOW);
  digitalWrite(PotmeterEnable, LOW);
}

void maintenance_state_update() {
  char key = keypad.getKey();
  switch(key) {
    case '1': digitalWrite(AirAssistPin, !digitalRead(AirAssistPin)); break;
    case '2': digitalWrite(WaterCoolerPin, !digitalRead(WaterCoolerPin)); break;
    case '3': digitalWrite(VentilationPin, !digitalRead(VentilationPin)); break;
    case '4': digitalWrite(LaserEnablePin, !digitalRead(LaserEnablePin));
              digitalWrite(LaosEnablePin, !digitalRead(LaosEnablePin)); break;
    case '5': digitalWrite(TestfireEnablePin, !digitalRead(TestfireEnablePin));
              digitalWrite(PotmeterEnable, !digitalRead(PotmeterEnable)); break;
    case '9': digitalWrite(AirAssistPin, HIGH);
              digitalWrite(WaterCoolerPin, HIGH);
              digitalWrite(VentilationPin, HIGH);
              digitalWrite(LaserEnablePin, HIGH);
              digitalWrite(LaosEnablePin, HIGH);
              digitalWrite(TestfireEnablePin, LOW);
              digitalWrite(PotmeterEnable, LOW);
              digitalWrite(LightsPin, LOW);
              break;
    case '0': digitalWrite(AirAssistPin, LOW);
              digitalWrite(WaterCoolerPin, LOW);
              digitalWrite(VentilationPin, LOW);
              digitalWrite(LaserEnablePin, LOW);
              digitalWrite(LaosEnablePin, LOW);
              digitalWrite(TestfireEnablePin, LOW);
              digitalWrite(PotmeterEnable, LOW);
              digitalWrite(LightsPin, HIGH);
              break;
    case '#':
      fsm.transitionTo(unlockedState);
      break;
  }

  maintenance_update_external_devices();
}

