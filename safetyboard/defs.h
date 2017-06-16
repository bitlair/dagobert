#include <Arduino.h>

static int WaterflowMeterPin = 2;

static int IButtonReaderPin = 3;
static int IButtonRedLedPin = 4;
static int IButtonGreenLedPin = 5;

static int TemperatureSensorPin = 6;

static int LaserEnableSignalPin = 11;

static int AirAssistPin = 52;
static int VentilationPin = 53;
static int WaterCoolerPin = 51;
static int LaserEnablePin = 50;
static int LightsPin = 49;
static int TestfireEnablePin = 47;
static int PotmeterEnable = 46;
static int LaosEnablePin = 44;

#define AUTHENTICATION_TIMEOUT 5000
#define TEMPERATURE_SENSOR_INTERVAL 10000
#define JOB_END_DELAY 30000
#define IDLE_TIMEOUT 300000
#define MINIMUM_VENTILATION_TIME 120000
#define WATERFLOW_SENSOR_INTERVAL 10000
#define LASER_KEEPALIVE_INTERVAL 5000

struct UserData {
  int maintenanceAllowed;
  int centsPerMinute;
  char accountName[256];

  long costOfLastJob = 0;
  long totalCost = 0;
};


