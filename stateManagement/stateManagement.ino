#include <Servo.h>
#include <SimpleExpressions.h>

Servo neck;
Servo head;

const int headStart = 50;
const int headStop = 130;

const int neckStart = 45;
const int neckStop = 120;

const int magnetSensorPin = 8;
const int shakeSensorPin = 9;
const int buzzer = 10; //buzzer to arduino pin 10
const int trigPin = 11;      // Trigger
const int echoPin = 12;      // Echo
const int ledPin = 13;

long duration, cm, inches;

enum State
{
  START,
  LONELY,
  FEAR,
  LOVE
};

enum Sensor
{
  NONE,
  EVE_SENSOR,
  DISTANCE_SENSOR_SHORT,
  DISTANCE_SENSOR_LONG,
  SHAKE_SENSOR
};

const int distanceShortThreshold = 7;

void state_machine_run(Sensor sensor);
void normal();
void lonely();
void fear();
void love();
void changeState(State nextState);
void playWalleSound();
void playWhineSound();
void ledPulseRed();
Sensor readSensor();

State state = START;

int lastInteraction = 0;
int startNewStateMillis = 0;

void setup()
{
  //Serial Port begin
  Serial.begin(9600);

  SimpleExpressions.init(ledPin, buzzer);


  neck.attach(10);
  head.attach(11);

  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(shakeSensorPin, INPUT);
  pinMode(magnetSensorPin, INPUT);
}

void loop()
{
  state_machine_run(readSensor());
  delay(10);
}

void state_machine_run(Sensor sensor)
{
  switch (state)
  {
  case START:
    if (sensor == DISTANCE_SENSOR_LONG)
    {
      lastInteraction = millis;
    }
    else if (sensor == SHAKE_SENSOR || sensor == DISTANCE_SENSOR_SHORT)
    {
      changeState(FEAR);
    }
    else if (sensor == EVE_SENSOR)
    {
      changeState(LOVE);
    }
    else
    {
      normal();
    }
    break;

  case LONELY:
    if (sensor == NONE)
    {
      lonely();
    }
    else
    {
      changeState(START);
    }
    break;

  case FEAR:
    if (sensor == SHAKE_SENSOR || sensor == DISTANCE_SENSOR_SHORT)
    {
      fear();
    }
    else
    {
      changeState(START);
    }
    break;

  case LOVE:
    if (sensor == EVE_SENSOR)
    {
      love();
    }
    else
    {
      changeState(START);
    }
    break;
  }
}

void changeState(State s)
{
  state = s;
  startNewStateMillis = millis;
}

void normal()
{
  const int length = 10 * 1000;
  const int pause = 10 * 1000;
  if (millis % (length + pause) > length)
  {
    head(calculateServoPosition(length, headStart, headStop));
  }
  if (millis % (length + pause / 2) < 20 && millis - lastInteraction > 1000)
  {
    lastInteraction = millis;
    playWalleSound();
  }
}

void lonely()
{
  const int time = 5000;
  head(calculateServoPosition(time, headStart, headStop));
  neck(calculateServoPosition(time, neckStart, neckStop));
}

void fear()
{
  playWhineSound();
  // shake
}

void love()
{
  ledPulseRed();
}

Sensor readSensor()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 2) * 0.0343;
  Serial.print(cm);
  Serial.println();

  if (cm < distanceShortThreshold)
  {
    return DISTANCE_SENSOR_SHORT;
  } else if (cm < 100) {
    return DISTANCE_SENSOR_LONG
  }

  // read Eve sensor

  // read shake sensor

  return NONE
}

int calculateServoPosition(duration, start, stop)
{
  return (int)(sin(millis * 2 / duration) + start) * stop;
}

void playWalleSound() {
  SimpleExpressions.playSound(0);
}

void playWhineSound() {
  SimpleExpressions.playSound(1);
}

void ledPulseRed() {
}
}
