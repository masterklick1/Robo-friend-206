// Eye renderer: pure C++ software rasteriser producing RGB565 pixels.
// No hardware dependencies -> can be previewed/tested on a PC.
// The eyes are drawn analytically (rounded-rect signed distance + lid cuts),
// anti-aliased, into a horizontal band of the 410x502 screen.
#pragma once
#include <stdint.h>
#include "mood_engine.h"

namespace rf {

class Eyes {
 public:
  static const int kScreenW = 410;
  static const int kBandY0 = 114;   // must be even (AMOLED needs even coordinates)
  static const int kBandH = 276;    // must be even
  static const int kPixels = kScreenW * kBandH;

  Eyes();

  void setMood(Mood m);                 // smooth transition
  void snapToMood(Mood m);              // instant (used by preview tool)
  void lookAt(float nx, float ny);      // -1..1, overrides idle wandering
  void releaseLook();
  void update(uint32_t now_ms);         // advance animation
  void render(uint16_t* band) const;    // writes kPixels RGB565 values

  // for preview/tests
  void setBlink(float openFactor) { forcedOpen_ = openFactor; useForcedOpen_ = true; }
  void clearBlinkOverride() { useForcedOpen_ = false; }
  void setGaze(float px, float py) { gx_ = px; gy_ = py; }

 private:
  struct Shape {
    float w, h, r;
    float lidTop, slant, lidBottom;
    float heart, open, asym;
    float cr, cg, cb;
  };

  static Shape targetFor(Mood m);
  void drawEye(uint16_t* band, float cx, float cy, bool left, float scale) const;
  void drawHeart(uint16_t* band, float cx, float cy, float scale) const;
  float rnd();
  void scheduleBlink(uint32_t now_ms);
  void scheduleSaccade(uint32_t now_ms);

  Mood mood_ = Mood::Neutral;
  Shape cur_;
  Shape tgt_;

  float gx_ = 0, gy_ = 0;           // current gaze offset in px
  float gtx_ = 0, gty_ = 0;         // gaze target in px
  bool lookOverride_ = false;
  float lookX_ = 0, lookY_ = 0;

  uint32_t last_ = 0;
  bool started_ = false;
  uint32_t nextBlink_ = 0;
  uint32_t blinkStart_ = 0;
  bool blinking_ = false;
  uint32_t nextSaccade_ = 0;
  float blinkFactor_ = 1.0f;
  float time_s_ = 0;
  float pulse_ = 1.0f;

  bool useForcedOpen_ = false;
  float forcedOpen_ = 1.0f;
  uint32_t rng_ = 0x1234ABCDu;
};

}  // namespace rf
