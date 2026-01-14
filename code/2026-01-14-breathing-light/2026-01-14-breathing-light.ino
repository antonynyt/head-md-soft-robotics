#include <Servo.h>
#include "SPI.h"
#include "Adafruit_WS2801.h"

Servo myservo;

// ----- PINS -----
uint8_t dataPin  = 2;
uint8_t clockPin = 3;

Adafruit_WS2801 strip = Adafruit_WS2801((uint16_t)7, (uint16_t)7, dataPin, clockPin);

// ----- SERVO -----
#define DEVIATION 40
int START_ANGLE = 20 + DEVIATION;
int END_ANGLE   = 60 + DEVIATION;

int pos = START_ANGLE;
int direction = 1;

// ----- TIMING -----
unsigned long lastServoUpdate = 0;
unsigned long lastLedUpdate   = 0;

// ----- SPEED -----
int speedMs = 20;   // updated from potentiometer

float dangerSmoothed = 0.0;

void setup() {
  pinMode(A0, INPUT);
  myservo.attach(6);

  strip.begin();
  strip.show(); // lights off

  Serial.begin(9600);
}

void loop() {
  // Read air (A0)
  int airQuality = analogRead(A0);

  Serial.println(analogRead(A0));
  speedMs = map(airQuality, 80, 350, 20, 5);
  speedMs = constrain(speedMs, 5, 20);
  

  updateBreathingLEDs();
  updateServo();
}

// ================= SERVO (non-blocking) =================
void updateServo() {
  unsigned long now = millis();

  if (now - lastServoUpdate >= speedMs) {
    lastServoUpdate = now;

    pos += direction;

    if (pos >= END_ANGLE || pos <= START_ANGLE) {
      direction *= -1;
    }

    myservo.write(pos);
  }
}

// Base colors (no brightness applied)
const float BASE_GREEN_R = 0.1;
const float BASE_GREEN_G = 0.5;
const float BASE_GREEN_B = 0.1;

const float BASE_RED_R   = 0.6;
const float BASE_RED_G   = 0.05;
const float BASE_RED_B   = 0.02;


// ================= LED WITH EASING =================
void updateBreathingLEDs() {
  unsigned long now = millis();

  if (now - lastLedUpdate >= speedMs) {
    lastLedUpdate = now;

    // -------- Servo easing --------
    float t = (float)(pos - START_ANGLE) / (float)(END_ANGLE - START_ANGLE);
    t = constrain(t, 0.0, 1.0);
    float eased = easeInOut(t);

    uint8_t brightness = (uint8_t)(
      255 + eased * (80 - 255)
    );
    brightness = constrain(brightness, 60, 255);

    // -------- Danger smoothing --------
    float dangerTarget = (float)(20 - speedMs) / (20 - 10);
    dangerTarget = constrain(dangerTarget, 0.0, 1.0);

    dangerSmoothed += (dangerTarget - dangerSmoothed) * 0.08;
    float danger = easeInOut(dangerSmoothed);

    // -------- Color blend (NO brightness yet) --------
    float r = BASE_GREEN_R + danger * (BASE_RED_R - BASE_GREEN_R);
    float g = BASE_GREEN_G + danger * (BASE_RED_G - BASE_GREEN_G);
    float b = BASE_GREEN_B + danger * (BASE_RED_B - BASE_GREEN_B);

    // -------- Apply brightness ONCE --------
    uint8_t R = r * brightness;
    uint8_t G = g * brightness;
    uint8_t B = b * brightness;

    uint32_t color = Color(R, G, B);

    for (int i = 13; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
  }
}



// ================= HELPERS =================
uint32_t Color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

float easeInOut(float t) {
  // t must be between 0.0 and 1.0
  return t * t * (3 - 2 * t);
}
