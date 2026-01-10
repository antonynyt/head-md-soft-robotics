#include <Servo.h>

Servo myservo;
int pos = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT);
  myservo.attach(6);
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  if (analogRead(0) > 200) {
    myservo.write(180);
    
  } else {
    for (pos = 10; pos <=50; pos++) {
      myservo.write(pos);
      delay(15);
    }
    for (pos = 50; pos >=10; pos--) {
      myservo.write(pos);
      delay(15);
    }
  }

  Serial.println(analogRead(0));

  delay(5);

}
