#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <SimpleExpressions.h>

Servo neck;
Servo head;

const int headStart = 80;
const int headStop = 100;

const int neckStart = 35;
const int neckStop = 120;

const int magnetSensorPin = A0;
const int buzzer = 5; // Buzzer to arduino pin 10
const int trigPin = 11;      // Trigger
const int echoPin = 12;      // Echo
const int ledPin = 4;

const int NUMPIXELS = 12;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_RGBW + NEO_KHZ800);


enum State
{
  START, //0
  LONELY, //1
  FEAR,  //2
  LOVE  //3
};

enum Sensor
{
  NONE,
  EVE_SENSOR,
  DISTANCE_SENSOR_SHORT,
  DISTANCE_SENSOR_LONG,
};

const int distanceShortThreshold = 15;
const int distanceLongThreshold = 100;

void state_machine_run(Sensor sensor);
void normal();
void lonely();
void fear();
void love();
void changeState(State nextState);
void playWhineSound();
Sensor readSensor();

State state = START;

long unsigned lastInteraction = 0;
int startNewStatemillis = 0;

long fading = 0;

int counterShort = 0;
int counterLong = 0;
int lastCm = 0;

void setup()
{
  //Serial Port begin
  Serial.begin(9600);

  pixels.begin(); // This initializes the NeoPixel library.

  SimpleExpressions.init(ledPin, buzzer);

  neck.attach(9);
  head.attach(10);

  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(magnetSensorPin, INPUT);
}

double neckRotation = 0;
double headRotation = 0;

long counter = 0;

void loop()
{
  state_machine_run(readSensor());
}

void state_machine_run(Sensor sensor)
{
  switch (state)
  {
    case START:
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
      }
      pixels.show();
      if (sensor == DISTANCE_SENSOR_LONG)
      {
        lastInteraction = millis();
        uint32_t rand = pixels.Color(0, random(200, 255), random(100), random(100));
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, rand); // Red color.
        }
        pixels.show();
        delay(100);
      }
      else if (sensor == DISTANCE_SENSOR_SHORT)
      {
        changeState(FEAR);
        SimpleExpressions.playSound(9);
      }
      else if (sensor == EVE_SENSOR)
      {
        changeState(LOVE);
        SimpleExpressions.playSound(12);
      }
      else if (millis() - lastInteraction > 10000) {
        changeState(LONELY);
        SimpleExpressions.playSound(16);
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 255, 0)); // Red color.
        }
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
      else
      {
        normal();
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
      if (sensor == DISTANCE_SENSOR_SHORT)
      {
        fear();
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
  startNewStatemillis = millis();
  lastInteraction = millis();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
  }
  pixels.show();
}

void normal()
{
  delay(70);
}

void lonely()
{
  Serial.println(calculateServoPosition(neckRotation, neckStart, neckStop));
  neck.write(calculateServoPosition(neckRotation, neckStart, neckStop));
  head.write(calculateServoPosition(headRotation, headStart, headStop));
  neckRotation += 0.005;
  headRotation += 0.01;
}

void fear()
{
  for (int i = 0; i < NUMPIXELS; i++) {
    if (fading % 2 == 0) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0, 0)); // Red color.
    }
    else {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
    }
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  fading += 1;
  delay(50);
}

void love()
{
  long redValue = fading;
  for (int i = 0; i < NUMPIXELS; i++) {
    if ((fading / 255) % 2 == 0) {
      redValue = 255 - fading % 255;
    }
    else {
      redValue = fading % 255;
    }
    pixels.setPixelColor(i, pixels.Color(0, redValue, 0, 0)); // Red color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  fading += 1;
}

Sensor readSensor()
{
  // read Eve sensor
  int magnetVal = analogRead(magnetSensorPin);
  if (magnetVal <= 300) {
    return EVE_SENSOR;
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);


  long duration = pulseIn(echoPin, HIGH, 4000);
  long cm = (duration / 2) * 0.0343;

  if (cm < distanceShortThreshold && cm > 3)
  {
    counterShort += 1;
    counterLong = 0;
    if (counterShort > 5) {
      return DISTANCE_SENSOR_SHORT;
    }
  } else if (cm < distanceLongThreshold && cm > distanceShortThreshold)
  {
    counterLong += 1;
    counterShort = 0;
    if (counterLong > 5)
      return DISTANCE_SENSOR_LONG;
  } else {
    counterShort = 0;
    counterLong = 0;
  }

  // read shake sensor

  return NONE;
}

double calculateServoPosition(double duration, int start, int stopR)
{
  return (sin(duration) + 1) * (stopR - start) / 2 + start;
}
