// Touch gesture detector: pure C++. Turns raw (down, x, y) samples into
// poke / stroke events. Call update() with every touch poll.
#pragma once
#include <stdint.h>

namespace rf {

struct GestureOut {
  bool poke = false;
  bool strokeBegin = false;
  bool strokeEnd = false;
};

class GestureDetector {
 public:
  GestureOut update(bool down, int x, int y, uint32_t now_ms);
  bool isDown() const { return down_; }
  bool isStroking() const { return stroking_; }
  int x() const { return x_; }
  int y() const { return y_; }

 private:
  bool down_ = false;
  bool stroking_ = false;
  uint32_t t0_ = 0;
  uint32_t lastSeen_ = 0;
  int x0_ = 0, y0_ = 0, x_ = 0, y_ = 0;
  int travel_ = 0;
};

}  // namespace rf
