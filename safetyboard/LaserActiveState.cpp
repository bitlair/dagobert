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

long jobStartTime;
int jobState;
long lastKeepAliveTime;

void laser_active_state_enter() {
  digitalWrite(IButtonGreenLedPin, HIGH);

  lcd.clear();
  lcd.print("Lasercutter Active");
  lcd.setCursor(0, 1);
  lcd.print(loggedInUser.accountName);

  jobStartTime = millis();
  jobState = 0;

  lastKeepAliveTime = millis();
  
  Serial.println("S");
}

void laser_active_state_leave() {
  
}

void laser_active_state_update() {
  long expiredSeconds = (millis() - jobStartTime) / 1000;
  long roundedMinutes = ceil((float)expiredSeconds / 60.0);

  long price = loggedInUser.centsPerMinute * roundedMinutes;

  char data[21];
  sprintf(data, "%02ld:%02ld = E%ld.%02ld", expiredSeconds / 60, expiredSeconds % 60, price / 100, price % 100);
  
  lcd.setCursor(0, 3);
  lcd.print(data);
  
  if ((millis() - laserActiveTime) > JOB_END_DELAY) {
    long totalJobTime = millis() - jobStartTime - JOB_END_DELAY;
    long totalPrice = ceil((float)(totalJobTime / 1000) / 60.0) * loggedInUser.centsPerMinute;

    loggedInUser.costOfLastJob = totalPrice;
    loggedInUser.totalCost += totalPrice;

    Serial.print("E");
    Serial.println(totalJobTime);

    fsm.transitionTo(unlockedState);
  }

  if ((millis() - lastKeepAliveTime) > LASER_KEEPALIVE_INTERVAL) {
    Serial.print("K");
    Serial.println(millis() - jobStartTime);
    lastKeepAliveTime = millis();
  }
}

