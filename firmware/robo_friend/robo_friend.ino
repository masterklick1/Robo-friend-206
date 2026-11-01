// Robo Friend v0.1 - animated eyes with moods for the
// Waveshare ESP32-S3-Touch-AMOLED-2.06 (410x502 CO5300 AMOLED, FT3168 touch).
//
// Arduino IDE settings (same as Waveshare's own examples):
//   Board: ESP32S3 Dev Module | Flash Size: 16MB
//   Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS)
//   PSRAM: OPI PSRAM | USB CDC On Boot: Enabled
//
// Needs one library: "GFX Library for Arduino" with CO5300 support
// (use the copy bundled in Waveshare's repo - see README).

#include <Arduino.h>
#include <Wire.h>
#include "Arduino_GFX_Library.h"
#include "esp_heap_caps.h"

#include "board_pins.h"
#include "touch_ft3168.h"
#include "mood_engine.h"
#include "gesture.h"
#include "eyes.h"

static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);

// col_offset1 = 22 as in the manufacturer's examples
static Arduino_GFX *gfx = new Arduino_CO5300(
    bus, LCD_RESET, 0 /* rotation */, LCD_WIDTH, LCD_HEIGHT, 22, 0, 0, 0);

static TouchFT3168 touch;
static rf::MoodEngine mood;
static rf::GestureDetector gesture;
static rf::Eyes eyes;

static uint16_t *band = nullptr;       // RGB565 strip holding the eyes (PSRAM)
static uint32_t lastFrameMs = 0;
static uint32_t lastTouchMs = 0;
static rf::Mood lastMood = rf::Mood::COUNT;
static uint8_t curBrightness = 0;

static const uint8_t kBrightnessAwake = 200;
static const uint8_t kBrightnessAsleep = 25;

static void setBrightnessIfChanged(uint8_t b) {
  if (b != curBrightness) {
    curBrightness = b;
    gfx->setBrightness(b);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Robo Friend v0.1");

  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  setBrightnessIfChanged(kBrightnessAwake);

  band = (uint16_t *)heap_caps_malloc(rf::Eyes::kPixels * sizeof(uint16_t),
                                      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!band) {
    band = (uint16_t *)malloc(rf::Eyes::kPixels * sizeof(uint16_t));
  }
  if (!band) {
    Serial.println("Out of memory for frame band (is PSRAM set to OPI?)");
    while (true) delay(1000);
  }

  if (!touch.begin(IIC_SDA, IIC_SCL)) {
    Serial.println("Touch init failed (continuing without touch)");
  }

  uint32_t now = millis();
  mood.update(now);
  eyes.snapToMood(mood.mood());
}

void loop() {
  uint32_t now = millis();

  // ---- touch polling (~65 Hz) ----
  if (now - lastTouchMs >= 15) {
    lastTouchMs = now;
    int x = 0, y = 0;
    bool down = touch.read(x, y);
    rf::GestureOut g = gesture.update(down, x, y, now);

    if (g.poke) mood.event(rf::Event::Poke, now);
    if (g.strokeBegin) mood.event(rf::Event::StrokeBegin, now);
    if (g.strokeEnd) mood.event(rf::Event::StrokeEnd, now);

    if (gesture.isDown()) {
      // eyes follow the finger
      eyes.lookAt((gesture.x() - LCD_WIDTH * 0.5f) / (LCD_WIDTH * 0.5f),
                  (gesture.y() - LCD_HEIGHT * 0.5f) / (LCD_HEIGHT * 0.5f));
    } else {
      eyes.releaseLook();
    }
  }

  // ---- animation / render ----
  uint32_t interval = mood.asleep() ? 100 : 33;   // slow down while sleeping
  if (now - lastFrameMs >= interval) {
    lastFrameMs = now;

    mood.update(now);
    rf::Mood m = mood.mood();
    if (m != lastMood) {
      lastMood = m;
      Serial.print("mood: ");
      Serial.println(rf::moodName(m));
    }
    eyes.setMood(m);
    eyes.update(now);
    eyes.render(band);

    // x and y of the blit must be even for the AMOLED controller
    gfx->draw16bitRGBBitmap(0, rf::Eyes::kBandY0, band, rf::Eyes::kScreenW, rf::Eyes::kBandH);

    setBrightnessIfChanged(mood.asleep() ? kBrightnessAsleep : kBrightnessAwake);
  }
}
