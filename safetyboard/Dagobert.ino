#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include "FiniteStateMachine.h"
#include "LockedState.h"
#include "UnlockedState.h"
#include "MaintenanceState.h"
#include "defs.h"
#include "Temperature.h"
#include "LaserActiveState.h"
#include "FlowMeter.h"
#include "LoadingState.h"

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = { 
  { '1','2','3'  }  ,
  { '4','5','6'  }  ,
  { '7','8','9'  }  ,
  { '*','0','#'  } };

byte colPins[COLS] = { 30, 22, 26 };   
byte rowPins[ROWS] = { 24, 34, 28, 32 };

LiquidCrystal_I2C lcd(0x27, 20, 4);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); 

State loadingState = State(loading_state_enter, loading_state_update, loading_state_leave);
State lockedState = State(locked_state_enter, locked_state_update, locked_state_leave);
State unlockedState = State(unlocked_state_enter, unlocked_state_update, unlocked_state_leave);
State maintenanceState = State(maintenance_state_enter, maintenance_state_update, maintenance_state_leave);
State laserActiveState = State(laser_active_state_enter, laser_active_state_update, laser_active_state_leave);

FSM fsm = FSM(loadingState);

volatile long laserActiveTime;
long ventilationDisableTime;

UserData loggedInUser;

void setup() {
  attachInterrupt(0, waterPulseCounter, RISING);
  pinMode(WaterflowMeterPin, INPUT_PULLUP);
  
  pinMode(LaserEnableSignalPin, INPUT_PULLUP);
  
  pinMode(IButtonGreenLedPin, OUTPUT);
  pinMode(IButtonRedLedPin, OUTPUT);

  pinMode(AirAssistPin, OUTPUT);
  pinMode(VentilationPin, OUTPUT);
  pinMode(WaterCoolerPin, OUTPUT);
  pinMode(LaserEnablePin, OUTPUT);
  pinMode(LightsPin, OUTPUT);
  pinMode(TestfireEnablePin, OUTPUT);
  pinMode(PotmeterEnable, OUTPUT);
  pinMode(LaosEnablePin, OUTPUT);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  Serial.println("Booted");

  setupTemperatureSensor();
}


void loop() {
  updateWaterFlow();
  
  if (digitalRead(LaserEnableSignalPin)) {
    laserActiveTime = millis();
  }

  if ((ventilationDisableTime > 0) && (millis() > ventilationDisableTime)) {
    digitalWrite(VentilationPin, LOW);
    ventilationDisableTime = 0;
  }
  
  fsm.update();
  updateTemperatureReading();
}
