
#include <Servo.h>

Servo neck;  // create servo object to control a servo
Servo head;

const int headStart = 50;
const int headStop = 130;

const int neckStart = 45;
const int neckStop = 120;

void setup() {
  neck.attach(10);
  head.attach(11);
}

void loop() {
  int pos = 0;
  for (pos = headStart; pos <= headStop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    head.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = headStop; pos >= headStart; pos -= 1) { // goes from 180 degrees to 0 degrees
    head.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = neckStart; pos <= neckStop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    neck.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = neckStop; pos >= neckStart; pos -= 1) { // goes from 180 degrees to 0 degrees
    neck.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
