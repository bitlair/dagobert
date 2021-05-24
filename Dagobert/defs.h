#include <Arduino.h>

static int IButtonReaderPin = 3;
static int IButtonRedLedPin = 4;
static int IButtonGreenLedPin = 5;

static int LaserEmergencySignalPin = 9;
static int LaserEnableSignalPin = 10;

static int AirAssistPin = 52; // Bruin
static int VentilationPin = 53; // Paars
static int LaserEnablePin = 50; // Oranje
static int RuidaEnablePin = 44; // Groen

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
