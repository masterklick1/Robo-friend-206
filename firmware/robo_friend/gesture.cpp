#include "gesture.h"

namespace rf {

static const uint32_t kReleaseDebounceMs = 60;   // finger must be gone this long
static const uint32_t kHoldStrokeMs = 600;       // still hold => stroke
static const int kDragStrokePx = 60;             // drag distance => stroke
static const int kPokeMaxTravelPx = 40;
static const uint32_t kPokeMaxMs = 500;

static inline int iabs(int v) { return v < 0 ? -v : v; }

GestureOut GestureDetector::update(bool down, int x, int y, uint32_t now_ms) {
  GestureOut out;
  if (down) {
    lastSeen_ = now_ms;
    x_ = x; y_ = y;
    if (!down_) {
      down_ = true;
      stroking_ = false;
      t0_ = now_ms;
      x0_ = x; y0_ = y;
      travel_ = 0;
    } else {
      int dx = iabs(x - x0_), dy = iabs(y - y0_);
      int t = dx > dy ? dx : dy;
      if (t > travel_) travel_ = t;
    }
    uint32_t held = now_ms - t0_;
    if (!stroking_ && ((held >= kHoldStrokeMs && travel_ < kDragStrokePx) ||
                       (travel_ >= kDragStrokePx && held >= 150))) {
      stroking_ = true;
      out.strokeBegin = true;
    }
  } else if (down_ && (now_ms - lastSeen_) >= kReleaseDebounceMs) {
    uint32_t held = lastSeen_ - t0_;
    down_ = false;
    if (stroking_) {
      stroking_ = false;
      out.strokeEnd = true;
    } else if (held < kPokeMaxMs && travel_ < kPokeMaxTravelPx) {
      out.poke = true;
    }
  }
  return out;
}

}  // namespace rf
