#include "Adafruit_WS2801.h"
#include "SPI.h"
#include <math.h>

// ----- PINS -----
uint8_t dataPin  = 2;
uint8_t clockPin = 3;

// ----- PIXEL COUNT -----
#define NUM_PIXELS 20

Adafruit_WS2801 strip = Adafruit_WS2801(NUM_PIXELS, dataPin, clockPin);

// ---------- Helpers ----------
static inline float s01(float x) {               // sine 0..1
  return 0.5f + 0.5f * sinf(x);
}

static inline uint8_t clamp8(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

uint32_t Color(byte r, byte g, byte b) {
  uint32_t c = r;
  c <<= 8; c |= g;
  c <<= 8; c |= b;
  return c;
}

// Simple HSV -> RGB (h:0..360, s:0..1, v:0..1)
void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  while (h < 0) h += 360.0f;
  while (h >= 360.0f) h -= 360.0f;

  float c = v * s;
  float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
  float m = v - c;

  float rp=0, gp=0, bp=0;
  if      (h < 60)  { rp=c; gp=x; bp=0; }
  else if (h < 120) { rp=x; gp=c; bp=0; }
  else if (h < 180) { rp=0; gp=c; bp=x; }
  else if (h < 240) { rp=0; gp=x; bp=c; }
  else if (h < 300) { rp=x; gp=0; bp=c; }
  else              { rp=c; gp=0; bp=x; }

  r = clamp8((int)((rp + m) * 255.0f));
  g = clamp8((int)((gp + m) * 255.0f));
  b = clamp8((int)((bp + m) * 255.0f));
}

// ---------- Behavior tuning ----------
// Overall brightness / breathing
const float BASE_V    = 0.65f;   // brighter base
const float AMP_V     = 0.30f;   // breathing depth
const float BREATH_HZ = 0.08f;   // ~12.5s cycle

// Color movement
const float DRIFT_HZ  = 0.03f;
const float SPACING   = 18.0f;   // hue difference per pixel (degrees)

// Cold palette range (cyan/blue/violet)
const float HUE_COLD_MIN = 165.0f;
const float HUE_COLD_MAX = 285.0f;

// Saturation
const float SAT_BASE = 0.88f;
const float SAT_VAR  = 0.10f;

// Yellow flashes behavior:
// We alternate: 10s with flashes, then 10s without.
const uint32_t FLASH_WINDOW_MS = 10000; // 10 seconds ON
const uint32_t CYCLE_MS        = 20000; // 10s ON + 10s OFF

// Flash pulse speed (NOT super fast strobe):
// 1.2 = about 1.2 flashes per second when in the ON window.
// Increase to 2.0 for more frequent twitches; keep it moderate.
const float YFLASH_HZ = 5.0f;

// Yellow spark palette
const float YELLOW_HUE = 55.0f; // warm yellow-gold
const float YELLOW_SAT = 0.95f;

// How strong the yellow flash overrides the base color
// 0.0 = no yellow, 1.0 = fully yellow during flash peaks
const float YELLOW_MIX_STRENGTH = 0.75f;

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  uint32_t nowMs = millis();
  float t = nowMs * 0.001f;

  // Breathing brightness (0..1)
  float breath = BASE_V + AMP_V * s01(2.0f * (float)M_PI * BREATH_HZ * t);
  if (breath > 1.0f) breath = 1.0f;

  // Base hue drifting through cold spectrum
  float coldCenter = (HUE_COLD_MIN + HUE_COLD_MAX) * 0.5f;
  float coldSpan   = (HUE_COLD_MAX - HUE_COLD_MIN) * 0.5f;
  float hueBase = coldCenter + coldSpan * sinf(2.0f * (float)M_PI * DRIFT_HZ * t);

  // Determine if we're inside the "10 seconds of yellow flashes" window
  uint32_t phase = nowMs % CYCLE_MS;
  bool yellowWindow = (phase < FLASH_WINDOW_MS);

  // Yellow flash intensity (0..1), only active in yellowWindow
  float yPulse = 0.0f;
  if (yellowWindow) {
    // Make "quick flashes": sharp peaks via power curve
    float s = s01(2.0f * (float)M_PI * YFLASH_HZ * t); // 0..1
    yPulse = powf(s, 10.0f); // sharper peaks (bigger exponent = snappier flash)
  }

  for (int i = 0; i < strip.numPixels(); i++) {
    float local = i * SPACING;

    // Local hue wobble for organic movement
    float hue = hueBase
              + local
              + 12.0f * sinf(2.0f * (float)M_PI * 0.04f * t + i * 0.7f);

    // Saturation variation
    float sat = SAT_BASE + SAT_VAR * sinf(2.0f * (float)M_PI * 0.03f * t + i * 0.33f);
    if (sat < 0.0f) sat = 0.0f;
    if (sat > 1.0f) sat = 1.0f;

    // Base RGB from cold palette
    uint8_t r1, g1, b1;
    hsvToRgb(hue, sat, breath, r1, g1, b1);

    // Yellow flash RGB (same brightness, but yellow hue)
    uint8_t r2, g2, b2;
    hsvToRgb(YELLOW_HUE, YELLOW_SAT, breath, r2, g2, b2);

    // Mix: during flash peaks, blend toward yellow
    float mix = (yellowWindow ? (YELLOW_MIX_STRENGTH * yPulse) : 0.0f);

    int R = (int)((1.0f - mix) * r1 + mix * r2);
    int G = (int)((1.0f - mix) * g1 + mix * g2);
    int B = (int)((1.0f - mix) * b1 + mix * b2);

    strip.setPixelColor(i, Color(clamp8(R), clamp8(G), clamp8(B)));
  }

  strip.show();
  delay(20);
}
