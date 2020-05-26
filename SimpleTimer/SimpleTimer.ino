#include <SimpleTimer.h>

// the timer object
SimpleTimer timer;

// a function to be executed periodically
void repeatMe() {
  Serial.print(millis());
  Serial.print("  ");
  Serial.print(millis() / 1000);
  Serial.println("  3 second timer");
}

void RepeatTask() {
  Serial.print(millis());
  Serial.print("  ");
  Serial.print(millis() / 1000);
  Serial.println("  5 second timer");
}
 
void repeatMe2() {
  Serial.print(millis());
  Serial.print("  ");
  Serial.print(millis() / 1000);
  Serial.println("  Only once_1");
}


void RepeatTask2() {
  Serial.print(millis());
  Serial.print("  ");
  Serial.print(millis() / 1000);
  Serial.println("  Only once_2");
}

void DigitalClockDisplay() {
  Serial.print("Clock: ");
  Serial.println(millis() / 1000);
}

void setup() {
  Serial.begin(9600);
  timer.setTimeout(1000, repeatMe2);
  timer.setTimeout(6500, RepeatTask2);
  timer.setInterval(60000, RepeatTask);
  timer.setInterval(66500, repeatMe);
  //timer.setInterval(1000, DigitalClockDisplay);
  pinMode(13, OUTPUT);

}

void loop() {

  timer.run();
  digitalWrite(13, 1);
  delay(500);
  digitalWrite(13, 0);
  delay(500);
}




//#include <SimpleTimer.h>
//
//// There must be one global SimpleTimer object.
//// More SimpleTimer objects can be created and run,
//// although there is little point in doing so.
//SimpleTimer timer;
//
//// function to be called repeatedly
//void RepeatTask() {
//  Serial.println("15 second timer");
//}
//
//void Repeat() {
//  Serial.println("5 second timer");
//}
//
//// function to be called just once
//void OnceOnlyTask() {
//  Serial.println("This timer only triggers once");
//}
//
//// function to be called exactly 10 times
//void TenTimesTask() {
//  static int k = 0;
//  k++;
//  Serial.print("called ");
//  Serial.print(k);
//  Serial.println(" / 10 times.");
//}
//
//// print current arduino "uptime" on the serial port
//void DigitalClockDisplay() {
//  Serial.print("Clock: ");
//  int h,m,s;
//  s = millis() / 1000;
//  m = s / 60;
//  h = s / 3600;
//  s = s - m * 60;
//  m = m - h * 60;
//  Serial.print(h);
//  printDigits(m);
//  printDigits(s);
//  Serial.println();
//}
//
////
//// utility function for digital clock display:
//// prints preceding colon and leading 0
////
//void printDigits(int digits) {
//  Serial.print(":");
//  if(digits < 10)
//    Serial.print('0');
//  Serial.print(digits);
//}
//
//void setup() {
//  Serial.begin(9600);
//
//  // welcome message
//  Serial.println("SimpleTimer Example");
//  Serial.println("One timer is triggered every 15 seconds");
//  Serial.println("Another timer is set to trigger only once after 10 seconds");
//  Serial.println("Another timer is set to trigger 10 times");
//  Serial.println();
//
//  // timed actions setup
//  timer.setInterval(15000, RepeatTask);
//  timer.setInterval(5000, Repeat);
//  timer.setTimeout(10000, OnceOnlyTask);
//  timer.setInterval(1000, DigitalClockDisplay);
//  timer.setTimer(1100, TenTimesTask, 10);
//}
//
//void loop() {
//  // this is where the "polling" occurs
//  timer.run();
//}

