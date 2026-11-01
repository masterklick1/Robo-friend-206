#include "eyes.h"
#include <math.h>
#include <string.h>

namespace rf {

static const float kEyeCenterY = 250.0f;
static const float kEyeDX = 95.0f;          // distance of each eye from screen centre
static const float kGazeAmpX = 30.0f;
static const float kGazeAmpY = 24.0f;

static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline uint16_t rgb565(float r, float g, float b) {
  int ri = (int)clampf(r, 0, 255), gi = (int)clampf(g, 0, 255), bi = (int)clampf(b, 0, 255);
  return (uint16_t)(((ri & 0xF8) << 8) | ((gi & 0xFC) << 3) | (bi >> 3));
}

Eyes::Eyes() {
  tgt_ = targetFor(Mood::Neutral);
  cur_ = tgt_;
}

// ---- mood -> eye shape ------------------------------------------------------
//        w     h     r    lidTop slant  lidBot heart open  asym   R    G    B
Eyes::Shape Eyes::targetFor(Mood m) {
  switch (m) {
    case Mood::Happy:     return {132, 150, 58, 0.00f,  0.00f, 0.72f, 0, 1.00f, 0.00f, 110, 240, 170};
    case Mood::Sad:       return {122, 170, 56, 0.10f, -0.95f, 0.00f, 0, 1.00f, 0.00f, 120, 160, 255};
    case Mood::Angry:     return {130, 160, 40, 0.08f,  0.95f, 0.00f, 0, 1.00f, 0.00f, 255,  96,  64};
    case Mood::Sleepy:    return {124, 170, 56, 0.48f, -0.25f, 0.00f, 0, 1.00f, 0.00f,  90, 170, 210};
    case Mood::Sleeping:  return {130, 170, 70, 0.00f,  0.00f, 0.00f, 0, 0.05f, 0.00f,  55, 100, 130};
    case Mood::Surprised: return {146, 206, 73, 0.00f,  0.00f, 0.00f, 0, 1.00f, 0.00f, 220, 245, 255};
    case Mood::Curious:   return {124, 176, 58, 0.00f,  0.00f, 0.00f, 0, 1.00f, 0.20f, 190, 150, 255};
    case Mood::Love:      return {156, 156, 60, 0.00f,  0.00f, 0.00f, 1, 1.00f, 0.00f, 255,  92, 150};
    case Mood::Bored:     return {122, 168, 56, 0.42f,  0.00f, 0.00f, 0, 1.00f, 0.00f, 150, 162, 185};
    case Mood::Neutral:
    default:              return {124, 176, 58, 0.00f,  0.00f, 0.00f, 0, 1.00f, 0.00f,  96, 214, 255};
  }
}

void Eyes::setMood(Mood m) {
  if (m == mood_) return;
  mood_ = m;
  tgt_ = targetFor(m);
}

void Eyes::snapToMood(Mood m) {
  mood_ = m;
  tgt_ = targetFor(m);
  cur_ = tgt_;
}

void Eyes::lookAt(float nx, float ny) {
  lookOverride_ = true;
  lookX_ = clampf(nx, -1, 1);
  lookY_ = clampf(ny, -1, 1);
}

void Eyes::releaseLook() { lookOverride_ = false; }

float Eyes::rnd() {
  rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
  return (float)(rng_ & 0xFFFFFF) / 16777216.0f;
}

void Eyes::scheduleBlink(uint32_t now_ms) {
  float lo = 2200, hi = 5500;
  if (mood_ == Mood::Sleepy || mood_ == Mood::Bored) { lo = 1500; hi = 3500; }
  nextBlink_ = now_ms + (uint32_t)(lo + rnd() * (hi - lo));
}

void Eyes::scheduleSaccade(uint32_t now_ms) {
  float lo = 1200, hi = 3500, ax = 1.0f, ay = 0.6f, by = 0.0f;
  switch (mood_) {
    case Mood::Curious:   lo = 600;  hi = 1600; ax = 1.3f; ay = 0.8f; break;
    case Mood::Happy:     lo = 1500; hi = 3000; ax = 0.8f; ay = 0.5f; break;
    case Mood::Sad:       lo = 2000; hi = 4000; ax = 0.4f; ay = 0.2f; by = 0.5f; break;
    case Mood::Sleepy:    lo = 3000; hi = 6000; ax = 0.3f; ay = 0.2f; by = 0.2f; break;
    case Mood::Bored:     lo = 2500; hi = 5000; ax = 1.0f; ay = 0.0f; break;
    case Mood::Angry:     lo = 2500; hi = 5000; ax = 0.1f; ay = 0.05f; break;
    case Mood::Love:      lo = 1500; hi = 2500; ax = 0.3f; ay = 0.3f; break;
    case Mood::Surprised:
    case Mood::Sleeping:  ax = 0; ay = 0; lo = 1500; hi = 3000; break;
    default: break;
  }
  gtx_ = (rnd() * 2 - 1) * ax * kGazeAmpX;
  gty_ = ((rnd() * 2 - 1) * ay + by) * kGazeAmpY;
  nextSaccade_ = now_ms + (uint32_t)(lo + rnd() * (hi - lo));
}

void Eyes::update(uint32_t now_ms) {
  if (!started_) {
    started_ = true;
    last_ = now_ms;
    scheduleBlink(now_ms);
    scheduleSaccade(now_ms);
  }
  float dt = (float)(uint32_t)(now_ms - last_) / 1000.0f;
  if (dt > 0.1f) dt = 0.1f;
  last_ = now_ms;
  time_s_ += dt;

  // smooth shape / colour toward the mood target
  float k = 1.0f - expf(-dt * 11.0f);
  float kc = 1.0f - expf(-dt * 6.0f);
  cur_.w += (tgt_.w - cur_.w) * k;
  cur_.h += (tgt_.h - cur_.h) * k;
  cur_.r += (tgt_.r - cur_.r) * k;
  cur_.lidTop += (tgt_.lidTop - cur_.lidTop) * k;
  cur_.slant += (tgt_.slant - cur_.slant) * k;
  cur_.lidBottom += (tgt_.lidBottom - cur_.lidBottom) * k;
  cur_.heart += (tgt_.heart - cur_.heart) * k;
  cur_.open += (tgt_.open - cur_.open) * (1.0f - expf(-dt * 8.0f));
  cur_.asym += (tgt_.asym - cur_.asym) * k;
  cur_.cr += (tgt_.cr - cur_.cr) * kc;
  cur_.cg += (tgt_.cg - cur_.cg) * kc;
  cur_.cb += (tgt_.cb - cur_.cb) * kc;

  // gaze
  if (lookOverride_) {
    gtx_ = lookX_ * kGazeAmpX;
    gty_ = lookY_ * kGazeAmpY;
  } else if ((int32_t)(now_ms - nextSaccade_) >= 0) {
    scheduleSaccade(now_ms);
  }
  float kg = 1.0f - expf(-dt * 14.0f);
  gx_ += (gtx_ - gx_) * kg;
  gy_ += (gty_ - gy_) * kg;

  // blinking
  bool canBlink = mood_ != Mood::Sleeping && mood_ != Mood::Love;
  float dur = (mood_ == Mood::Sleepy) ? 420.0f : 180.0f;
  if (!blinking_ && canBlink && (int32_t)(now_ms - nextBlink_) >= 0) {
    blinking_ = true;
    blinkStart_ = now_ms;
  }
  if (blinking_) {
    float t = (float)(uint32_t)(now_ms - blinkStart_) / dur;
    if (t >= 1.0f) {
      blinking_ = false;
      blinkFactor_ = 1.0f;
      scheduleBlink(now_ms);
    } else {
      blinkFactor_ = (t < 0.4f) ? 1.0f - t / 0.4f : (t - 0.4f) / 0.6f;
    }
  } else {
    blinkFactor_ = 1.0f;
  }

  // breathing / heartbeat
  if (mood_ == Mood::Love) {
    float s = sinf(time_s_ * 2.0f * 3.14159265f * 1.4f);
    pulse_ = 1.0f + 0.07f * (s > 0 ? s : 0);
  } else {
    pulse_ = 1.0f + 0.02f * sinf(time_s_ * 2.0f * 3.14159265f * 0.3f);
  }
}

void Eyes::render(uint16_t* band) const {
  memset(band, 0, sizeof(uint16_t) * kPixels);
  float cxL = kScreenW * 0.5f - kEyeDX;
  float cxR = kScreenW * 0.5f + kEyeDX;
  if (cur_.heart > 0.5f) {
    drawHeart(band, cxL, kEyeCenterY, 1.0f);
    drawHeart(band, cxR, kEyeCenterY, 1.0f);
  } else {
    drawEye(band, cxL, kEyeCenterY, true, 1.0f);
    drawEye(band, cxR, kEyeCenterY, false, 1.0f - cur_.asym);
  }
}

void Eyes::drawEye(uint16_t* band, float cx, float cy, bool left, float scale) const {
  float open = (useForcedOpen_ ? forcedOpen_ : blinkFactor_) * cur_.open;
  float hw = cur_.w * 0.5f * scale * pulse_;
  float hhFull = cur_.h * 0.5f * scale * pulse_;
  float hh = hhFull * open;
  if (hh < 2.0f) hh = 2.0f;                       // closed eye = thin line
  float rr = cur_.r * scale;
  if (rr > hw) rr = hw;
  if (rr > hh) rr = hh;

  float ex = cx + gx_;
  float ey = cy + gy_;

  const float lidTop = cur_.lidTop;
  const float slantPx = cur_.slant * hh * 0.9f;          // vertical shift across half width
  const float slope = slantPx / hw;
  const float invNorm = 1.0f / sqrtf(1.0f + slope * slope);
  const float lidBot = cur_.lidBottom;
  const float ellThick = 2.0f * hh * (1.0f - lidBot);          // crescent thickness at centre
  const float ellYc = hh;                                       // ellipse centre y (eye bottom)
  const float ellRy = ellYc - (-hh + ellThick);
  const float ellRx = hw * 1.15f;
  const float ellK = (ellRx < ellRy ? ellRx : ellRy) * 0.8f;

  bool hlOn = open > 0.6f && lidTop < 0.3f && lidBot < 0.2f;
  float hr1 = 0.17f * hw, hr2 = 0.08f * hw;
  float hx1 = -0.28f * hw, hy1 = -0.30f * hhFull;
  float hx2 = 0.28f * hw, hy2 = 0.34f * hhFull;

  int yMin = (int)floorf(ey - hh - 3), yMax = (int)ceilf(ey + hh + 3);
  int xMin = (int)floorf(ex - hw - 3), xMax = (int)ceilf(ex + hw + 3);
  if (yMin < kBandY0) yMin = kBandY0;
  if (yMax > kBandY0 + kBandH - 1) yMax = kBandY0 + kBandH - 1;
  if (xMin < 0) xMin = 0;
  if (xMax > kScreenW - 1) xMax = kScreenW - 1;

  for (int y = yMin; y <= yMax; y++) {
    float ly = (float)y + 0.5f - ey;
    float qy = fabsf(ly) - (hh - rr);
    uint16_t* row = band + (y - kBandY0) * kScreenW;
    for (int x = xMin; x <= xMax; x++) {
      float lx = (float)x + 0.5f - ex;
      float qx = fabsf(lx) - (hw - rr);
      float d;
      if (qx > 0 && qy > 0) d = sqrtf(qx * qx + qy * qy) - rr;
      else d = (qx > qy ? qx : qy) - rr;
      float cov = clampf(0.5f - d, 0, 1);
      if (cov <= 0) continue;

      // top lid (flat + slanted)
      float inner = left ? lx : -lx;
      float yTop = -hh + lidTop * 2.0f * hh + slope * inner;
      float covT = clampf(0.5f + (ly - yTop) * invNorm, 0, 1);
      if (covT < cov) cov = covT;

      // bottom lid: an ellipse rising from below is subtracted -> crescent ("^_^")
      if (lidBot > 0.01f) {
        float ny = (ly - ellYc) / ellRy;
        float nx = lx / ellRx;
        float f = sqrtf(nx * nx + ny * ny) - 1.0f;
        float covB = clampf(0.5f + f * ellK, 0, 1);
        if (covB < cov) cov = covB;
      }
      if (cov <= 0) continue;

      float r = cur_.cr, g = cur_.cg, b = cur_.cb;
      if (hlOn) {
        float dx = lx - hx1, dy = ly - hy1;
        if (fabsf(dx) < hr1 + 1 && fabsf(dy) < hr1 + 1) {
          float a = clampf(0.5f + (hr1 - sqrtf(dx * dx + dy * dy)), 0, 1) * 0.92f;
          r += (255 - r) * a; g += (255 - g) * a; b += (255 - b) * a;
        }
        dx = lx - hx2; dy = ly - hy2;
        if (fabsf(dx) < hr2 + 1 && fabsf(dy) < hr2 + 1) {
          float a = clampf(0.5f + (hr2 - sqrtf(dx * dx + dy * dy)), 0, 1) * 0.5f;
          r += (255 - r) * a; g += (255 - g) * a; b += (255 - b) * a;
        }
      }
      row[x] = rgb565(r * cov, g * cov, b * cov);
    }
  }
}

// Heart eyes: implicit curve (x^2+y^2-1)^3 - x^2*y^3 <= 0, 2x2 supersampled.
void Eyes::drawHeart(uint16_t* band, float cx, float cy, float scale) const {
  float half = cur_.w * 0.5f * scale * pulse_;
  float unit = half / 1.20f;                      // pixels per heart-unit
  float ex = cx + gx_ * 0.5f;
  float ey = cy + gy_ * 0.5f;
  int yMin = (int)floorf(ey - half - 2), yMax = (int)ceilf(ey + half + 2);
  int xMin = (int)floorf(ex - half - 2), xMax = (int)ceilf(ex + half + 2);
  if (yMin < kBandY0) yMin = kBandY0;
  if (yMax > kBandY0 + kBandH - 1) yMax = kBandY0 + kBandH - 1;
  if (xMin < 0) xMin = 0;
  if (xMax > kScreenW - 1) xMax = kScreenW - 1;
  const float off[2] = {0.25f, 0.75f};
  for (int y = yMin; y <= yMax; y++) {
    uint16_t* row = band + (y - kBandY0) * kScreenW;
    for (int x = xMin; x <= xMax; x++) {
      int hits = 0;
      for (int sy = 0; sy < 2; sy++) {
        for (int sx = 0; sx < 2; sx++) {
          float hx = ((float)x + off[sx] - ex) / unit;
          float hy = -((float)y + off[sy] - ey) / unit + 0.12f;
          float a = hx * hx + hy * hy - 1.0f;
          float f = a * a * a - hx * hx * hy * hy * hy;
          if (f <= 0) hits++;
        }
      }
      if (!hits) continue;
      float cov = hits * 0.25f;
      // soft inner glow: brighter toward the upper-left lobe
      float r = cur_.cr, g = cur_.cg, b = cur_.cb;
      float lx = ((float)x - ex) / half, ly = ((float)y - ey) / half;
      float glow = clampf(0.35f - (lx + 0.35f) * 0.25f - (ly + 0.35f) * 0.25f, 0, 0.35f);
      r += (255 - r) * glow; g += (255 - g) * glow; b += (255 - b) * glow;
      row[x] = rgb565(r * cov, g * cov, b * cov);
    }
  }
}

}  // namespace rf
