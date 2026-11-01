#include "mood_engine.h"

namespace rf {

static inline int32_t diff(uint32_t a, uint32_t b) { return (int32_t)(a - b); }
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

const char* moodName(Mood m) {
  switch (m) {
    case Mood::Neutral: return "neutral";
    case Mood::Happy: return "happy";
    case Mood::Sad: return "sad";
    case Mood::Sleepy: return "sleepy";
    case Mood::Sleeping: return "sleeping";
    case Mood::Surprised: return "surprised";
    case Mood::Angry: return "angry";
    case Mood::Curious: return "curious";
    case Mood::Love: return "love";
    case Mood::Bored: return "bored";
    default: return "?";
  }
}

MoodEngine::MoodEngine() {
  for (uint8_t i = 0; i < kHist; i++) { pokes_[i] = 0; shakes_[i] = 0; }
}

uint8_t MoodEngine::countRecent(const uint32_t* times, uint8_t n, uint32_t now_ms, uint32_t window_ms) const {
  uint8_t c = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (times[i] != 0 && diff(now_ms, times[i]) >= 0 && (uint32_t)diff(now_ms, times[i]) <= window_ms) c++;
  }
  return c;
}

void MoodEngine::startReaction(Mood m, uint32_t now_ms, uint32_t dur_ms) {
  hasReaction_ = true;
  reaction_ = m;
  reactionEnd_ = now_ms + dur_ms;
  hasQueued_ = false;
}

void MoodEngine::queueReaction(Mood m, uint32_t dur_ms) {
  hasQueued_ = true;
  queued_ = m;
  queuedDur_ = dur_ms;
}

void MoodEngine::touchInteraction(uint32_t now_ms) {
  lastInteract_ = now_ms;
  if (!started_) { started_ = true; lastUpdate_ = now_ms; }
}

void MoodEngine::wake(uint32_t now_ms) {
  if (sleeping_) {
    sleeping_ = false;
    if (energy_ < 35.0f) energy_ = 35.0f;
    startReaction(Mood::Surprised, now_ms, 1100);
  }
}

void MoodEngine::event(Event e, uint32_t now_ms) {
  bool wasAsleep = sleeping_;
  touchInteraction(now_ms);
  wake(now_ms);

  switch (e) {
    case Event::Poke: {
      pokes_[pokeIdx_] = now_ms;
      pokeIdx_ = (pokeIdx_ + 1) % kHist;
      if (countRecent(pokes_, kHist, now_ms, 3000) >= 5) {
        happy_ = clampf(happy_ - 8.0f, 0, 100);
        startReaction(Mood::Angry, now_ms, 2500);
      } else if (!wasAsleep) {
        happy_ = clampf(happy_ + 4.0f, 0, 100);
        startReaction(Mood::Surprised, now_ms, 300);
        queueReaction(Mood::Happy, 1500);
      }
      break;
    }
    case Event::StrokeBegin:
      stroking_ = true;
      break;
    case Event::StrokeEnd:
      if (stroking_) {
        stroking_ = false;
        happy_ = clampf(happy_ + 6.0f, 0, 100);
        startReaction(Mood::Happy, now_ms, 2500);
      }
      break;
    case Event::Shake: {
      shakes_[shakeIdx_] = now_ms;
      shakeIdx_ = (shakeIdx_ + 1) % kHist;
      happy_ = clampf(happy_ - 3.0f, 0, 100);
      if (countRecent(shakes_, kHist, now_ms, 2000) >= 3) startReaction(Mood::Angry, now_ms, 2500);
      else if (!wasAsleep) startReaction(Mood::Surprised, now_ms, 800);
      break;
    }
    case Event::LoudSound:
      if (!wasAsleep) startReaction(Mood::Surprised, now_ms, 600);
      break;
  }
  update(now_ms);
}

Mood MoodEngine::baseMood(uint32_t now_ms) const {
  if (sleeping_) return Mood::Sleeping;
  float idle_s = (float)(uint32_t)diff(now_ms, lastInteract_) / 1000.0f;
  if (energy_ < 20.0f || idle_s >= 180.0f) return Mood::Sleepy;
  if (idle_s >= 90.0f) return Mood::Bored;
  if (happy_ <= 28.0f) return Mood::Sad;
  if (happy_ >= 72.0f) return Mood::Happy;
  if (idle_s >= 25.0f) return Mood::Curious;
  return Mood::Neutral;
}

void MoodEngine::update(uint32_t now_ms) {
  if (!started_) { started_ = true; lastUpdate_ = now_ms; lastInteract_ = now_ms; }
  float dt = (float)(uint32_t)diff(now_ms, lastUpdate_) / 1000.0f;
  if (dt > 1.0f) dt = 1.0f;
  if (dt < 0.0f) dt = 0.0f;
  lastUpdate_ = now_ms;

  float idle_s = (float)(uint32_t)diff(now_ms, lastInteract_) / 1000.0f;

  if (sleeping_) {
    energy_ = clampf(energy_ + 1.5f * dt, 0, 100);   // rests while asleep; happiness frozen
  } else {
    energy_ = clampf(energy_ - 0.05f * dt, 0, 100);
    if (stroking_) {
      happy_ = clampf(happy_ + 8.0f * dt, 0, 100);
    } else if (idle_s > 120.0f) {
      happy_ = clampf(happy_ - 0.4f * dt, 15, 100);  // feels neglected
    } else if (happy_ > 55.0f) {
      happy_ = clampf(happy_ - 0.2f * dt, 55, 100);
    } else if (happy_ < 55.0f) {
      happy_ = clampf(happy_ + 0.2f * dt, 0, 55);
    }
    if (idle_s >= 240.0f || (energy_ < 10.0f && idle_s >= 60.0f)) sleeping_ = true;
  }

  // transient reactions
  if (hasReaction_ && diff(now_ms, reactionEnd_) >= 0) {
    if (hasQueued_) {
      hasReaction_ = true;
      reaction_ = queued_;
      reactionEnd_ = now_ms + queuedDur_;
      hasQueued_ = false;
    } else {
      hasReaction_ = false;
    }
  }

  if (stroking_) current_ = Mood::Love;
  else if (hasReaction_) current_ = reaction_;
  else current_ = baseMood(now_ms);
}

}  // namespace rf
