#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <SimpleExpressions.h>

Servo neck;
Servo head;

const int headStart = 50;
const int headStop = 130;

const int neckStart = 45;
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
};

const int distanceShortThreshold = 15;

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

void setup()
{
  //Serial Port begin
  Serial.begin(9600);
  
  pixels.begin(); // This initializes the NeoPixel library.

  SimpleExpressions.init(ledPin, buzzer);

  neck.attach(10);
  head.attach(11);

  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(magnetSensorPin, INPUT);
}

void loop()
{
  state_machine_run(readSensor());
}

void state_machine_run(Sensor sensor)
{
  switch (state)
  {
  case START:
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0,0));
    }
    pixels.show();
    if (sensor == DISTANCE_SENSOR_LONG)
    {
      lastInteraction = millis();
    }
    else if (sensor == DISTANCE_SENSOR_SHORT)
    {
      changeState(FEAR);
      SimpleExpressions.playSound(10);
    }
    else if (sensor == EVE_SENSOR)
    {
      changeState(LOVE);
      SimpleExpressions.playSound(12);
    }
    else if (millis() - startNewStatemillis > 5000) {
      changeState(LONELY);
      SimpleExpressions.playSound(16);
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
}

void normal()
{
  const int length = 10 * 1000;
  const int pause = 10 * 1000;
  if (millis() % (length + pause) > length)
  {
    //Serial.println(calculateServoPosition(length, headStart, headStop));
  }
  if (millis() % (length + pause / 2) < 20 && millis() - lastInteraction > 1000)
  {
    lastInteraction = millis();
  }
}

void lonely()
{
  const int time = 5000;
  //Serial.println(calculateServoPosition(time, headStart, headStop));
  //Serial.println(calculateServoPosition(time, neckStart, neckStop));
}

void fear()
{
  for(int i=0;i<NUMPIXELS;i++){
    if(fading % 2 == 0) {
      pixels.setPixelColor(i, pixels.Color(0,255,0,0)); // Red color.
    }
    else {
      pixels.setPixelColor(i, pixels.Color(0,0,0,0));
    }
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  fading += 1;
  delay(50);
}

void love()
{
  long redValue = fading;
  for(int i=0;i<NUMPIXELS;i++){
    if((fading / 255) % 2 == 0) {
      redValue = 255 - fading % 255;
    }
    else {
      redValue = fading % 255;
    }
     pixels.setPixelColor(i, pixels.Color(0,redValue,0,0)); // Red color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  fading += 1;
}

Sensor readSensor()
{
  // read Eve sensor
  int magnetVal = analogRead(magnetSensorPin);
  if(magnetVal <= 10) {
    return EVE_SENSOR;
  }
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long cm = (duration / 2) * 0.0343;

  if (cm < distanceShortThreshold)
  {
    return DISTANCE_SENSOR_SHORT;
  } else if (cm < 100) {
    return DISTANCE_SENSOR_LONG;
  }

  // read shake sensor

  return NONE;
}

int calculateServoPosition(int duration, int start, int stop)
{
  return (int)(sin(millis() * 2 / duration) + start) * stop;
}
