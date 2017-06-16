#define FLOW_INPUT_PIN     A2
#define LASER_ONOFF_PIN    A3
#define LASER_PWM_PIN      6
#define LASER_ENABLE_PIN   5
#define FLOW_OUTPUT_PIN    9

long lastFireTime;
long lastLaserChangeTime;

void setup() {
  pinMode(FLOW_INPUT_PIN, INPUT);
  pinMode(LASER_ONOFF_PIN, INPUT_PULLUP);
  //pinMode(LASER_PWM_PIN, OUTPUT);
  pinMode(LASER_ENABLE_PIN, OUTPUT);
  pinMode(FLOW_OUTPUT_PIN, OUTPUT);

  randomSeed(analogRead(0));
  Serial.begin(9600);
}

void loop() {
  int simulatedFlow = 1024 - analogRead(FLOW_INPUT_PIN);
  if (millis() - lastFireTime > (simulatedFlow * 2)) {
    digitalWrite(FLOW_OUTPUT_PIN, HIGH);
    delay(10);
    digitalWrite(FLOW_OUTPUT_PIN, LOW);

    lastFireTime = millis();
  }

  if(digitalRead(LASER_ONOFF_PIN)) {
    //analogWrite(LASER_PWM_PIN, 0);
    analogWrite(LASER_ENABLE_PIN, 255);

    lastLaserChangeTime = 0;
  } else {
    if (!lastLaserChangeTime || ((millis() - lastLaserChangeTime) > 30000)) {
      lastLaserChangeTime = millis();
      
      //analogWrite(LASER_PWM_PIN, random(50, 255));

      int fireUsingPWM = random(0, 2);
      if (!fireUsingPWM) {
        analogWrite(LASER_ENABLE_PIN, 0);
      } else {
        analogWrite(LASER_ENABLE_PIN, random(255));
      }
    }
  }
}
