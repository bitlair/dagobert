#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "defs.h"
#include "FiniteStateMachine.h"
#include <Keypad.h>

extern LiquidCrystal_I2C lcd;
extern UserData loggedInUser;
extern State lockedState;
extern FSM fsm;
extern Keypad keypad;
extern State maintenanceState;
extern State laserActiveState;
extern volatile long laserActiveTime;

long unlockedStateEnterTime;

void unlocked_state_enter() {
  unlockedStateEnterTime = millis();
  
  char data[21];

  digitalWrite(IButtonGreenLedPin, HIGH);

  lcd.clear();

  if (loggedInUser.costOfLastJob == 0) {
    lcd.print("Bitlair Lasercutter");
    lcd.setCursor(0, 1);
    lcd.print(loggedInUser.accountName);
    lcd.setCursor(0, 3);
  } else {
    lcd.print(loggedInUser.accountName);

    sprintf(data, "Last job: E%ld.%02ld", loggedInUser.costOfLastJob / 100, loggedInUser.costOfLastJob % 100);
    lcd.setCursor(0, 1);
    lcd.print(data);
  }
  
  int euro = loggedInUser.centsPerMinute / 100;
  sprintf(data, "E%d.%02d", euro, loggedInUser.centsPerMinute % 100);

  lcd.setCursor(0, 3);
  lcd.print(data);
  lcd.print(" p/m");

  lcd.setCursor(12, 3);
  lcd.print("* logout");

  if (loggedInUser.maintenanceAllowed) {
    lcd.setCursor(7, 2);
    lcd.print("# maintenance");
  }

  digitalWrite(AirAssistPin, HIGH);
  digitalWrite(VentilationPin, HIGH);
  digitalWrite(WaterCoolerPin, HIGH);
  digitalWrite(LaserEnablePin, HIGH);
  digitalWrite(LightsPin, LOW);
  digitalWrite(LaosEnablePin, HIGH);
}

void unlocked_state_leave() {
  digitalWrite(IButtonGreenLedPin, LOW);
}

void unlocked_state_update() {
  if ((millis() - laserActiveTime) < 250) {
    fsm.transitionTo(laserActiveState);  
  }

  if ((millis() - unlockedStateEnterTime) > IDLE_TIMEOUT) {
    fsm.transitionTo(lockedState);
  }
  
  char key = keypad.getKey();
  switch(key) {
    case '*':
      fsm.transitionTo(lockedState);
      break;
    case '#':
      if (loggedInUser.maintenanceAllowed) {
        fsm.transitionTo(maintenanceState);
      }
      break;
  }
}

