// Mood engine: pure C++ (no Arduino / hardware dependencies) so it can be
// unit-tested on a PC. Feed it events, call update() every frame, read mood().
#pragma once
#include <stdint.h>

namespace rf {

enum class Mood : uint8_t {
  Neutral = 0,
  Happy,
  Sad,
  Sleepy,
  Sleeping,
  Surprised,
  Angry,
  Curious,
  Love,
  Bored,
  COUNT
};

enum class Event : uint8_t {
  Poke,         // quick tap
  StrokeBegin,  // finger starts petting (long hold / drag)
  StrokeEnd,    // finger lifted after petting
  Shake,        // IMU shake (v0.2)
  LoudSound     // microphone spike (v0.3)
};

const char* moodName(Mood m);

class MoodEngine {
 public:
  MoodEngine();

  void event(Event e, uint32_t now_ms);
  void update(uint32_t now_ms);

  Mood mood() const { return current_; }
  bool asleep() const { return sleeping_; }
  bool stroking() const { return stroking_; }
  float happiness() const { return happy_; }  // 0..100
  float energy() const { return energy_; }    // 0..100
  uint32_t idleMs(uint32_t now_ms) const { return now_ms - lastInteract_; }

 private:
  void touchInteraction(uint32_t now_ms);
  void wake(uint32_t now_ms);
  void startReaction(Mood m, uint32_t now_ms, uint32_t dur_ms);
  void queueReaction(Mood m, uint32_t dur_ms);
  Mood baseMood(uint32_t now_ms) const;
  uint8_t countRecent(const uint32_t* times, uint8_t n, uint32_t now_ms, uint32_t window_ms) const;

  Mood current_ = Mood::Neutral;
  float happy_ = 60.0f;
  float energy_ = 80.0f;
  bool sleeping_ = false;
  bool stroking_ = false;

  uint32_t lastInteract_ = 0;
  uint32_t lastUpdate_ = 0;
  bool started_ = false;

  // transient reaction (+ one queued follow-up)
  bool hasReaction_ = false;
  Mood reaction_ = Mood::Neutral;
  uint32_t reactionEnd_ = 0;
  bool hasQueued_ = false;
  Mood queued_ = Mood::Neutral;
  uint32_t queuedDur_ = 0;

  static const uint8_t kHist = 6;
  uint32_t pokes_[kHist];
  uint32_t shakes_[kHist];
  uint8_t pokeIdx_ = 0;
  uint8_t shakeIdx_ = 0;
};

}  // namespace rf
