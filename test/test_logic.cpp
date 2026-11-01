// Host-side tests for mood engine + gesture detector.
//   g++ -std=c++17 -I../firmware/robo_friend test_logic.cpp ../firmware/robo_friend/mood_engine.cpp ../firmware/robo_friend/gesture.cpp -o test_logic && ./test_logic
#include <cstdio>
#include "mood_engine.h"
#include "gesture.h"
using namespace rf;

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)

static void run(MoodEngine& m, uint32_t from, uint32_t to) {
  for (uint32_t t = from; t <= to; t += 33) m.update(t);
}

int main() {
  // 1. starts neutral
  { MoodEngine m; m.update(0); CHECK(m.mood() == Mood::Neutral); }

  // 2. poke -> surprised -> happy -> back to neutral
  { MoodEngine m; m.update(0); m.event(Event::Poke, 1000);
    CHECK(m.mood() == Mood::Surprised);
    run(m, 1000, 1400); CHECK(m.mood() == Mood::Happy);
    run(m, 1400, 3500); CHECK(m.mood() == Mood::Neutral); }

  // 3. five fast pokes -> angry
  { MoodEngine m; m.update(0);
    for (int i = 0; i < 5; i++) m.event(Event::Poke, 1000 + i * 300);
    CHECK(m.mood() == Mood::Angry);
    run(m, 2200, 5000); CHECK(m.mood() != Mood::Angry); }

  // 4. stroke -> love, release -> happy afterglow
  { MoodEngine m; m.update(0); m.event(Event::StrokeBegin, 1000);
    run(m, 1000, 3000); CHECK(m.mood() == Mood::Love);
    float h = m.happiness(); CHECK(h > 70.0f);
    m.event(Event::StrokeEnd, 3000); CHECK(m.mood() == Mood::Happy); }

  // 5. idle timeline: curious -> bored -> sleepy -> sleeping, poke wakes
  { MoodEngine m; m.update(0);
    run(m, 0, 30000);  CHECK(m.mood() == Mood::Curious);
    run(m, 30000, 100000); CHECK(m.mood() == Mood::Bored);
    run(m, 100000, 200000); CHECK(m.mood() == Mood::Sleepy);
    run(m, 200000, 260000); CHECK(m.mood() == Mood::Sleeping); CHECK(m.asleep());
    m.event(Event::Poke, 261000); CHECK(m.mood() == Mood::Surprised); CHECK(!m.asleep()); }

  // 6. shaking 3x quickly -> angry
  { MoodEngine m; m.update(0);
    m.event(Event::Shake, 1000); m.event(Event::Shake, 1400); m.event(Event::Shake, 1800);
    CHECK(m.mood() == Mood::Angry); }

  // 7. gesture: quick tap = poke
  { GestureDetector g; int pokes = 0, sb = 0, se = 0;
    for (uint32_t t = 0; t < 1000; t += 20) {
      bool down = t >= 100 && t < 220;
      GestureOut o = g.update(down, 200, 250, t);
      pokes += o.poke; sb += o.strokeBegin; se += o.strokeEnd; }
    CHECK(pokes == 1 && sb == 0 && se == 0); }

  // 8. gesture: long hold = stroke begin + end, no poke
  { GestureDetector g; int pokes = 0, sb = 0, se = 0;
    for (uint32_t t = 0; t < 3000; t += 20) {
      bool down = t >= 100 && t < 1500;
      GestureOut o = g.update(down, 200, 250, t);
      pokes += o.poke; sb += o.strokeBegin; se += o.strokeEnd; }
    CHECK(pokes == 0 && sb == 1 && se == 1); }

  // 9. gesture: drag = stroke
  { GestureDetector g; int pokes = 0, sb = 0, se = 0;
    for (uint32_t t = 0; t < 2000; t += 20) {
      bool down = t >= 100 && t < 800;
      int x = 100 + (t > 100 ? (int)(t - 100) / 4 : 0);
      GestureOut o = g.update(down, x, 250, t);
      pokes += o.poke; sb += o.strokeBegin; se += o.strokeEnd; }
    CHECK(pokes == 0 && sb == 1 && se == 1); }

  // 10. one-sample flicker does not release a held touch
  { GestureDetector g; int pokes = 0, sb = 0, se = 0;
    for (uint32_t t = 0; t < 3000; t += 20) {
      bool down = (t >= 100 && t < 1500) && !(t == 600);  // single missed sample
      GestureOut o = g.update(down, 200, 250, t);
      pokes += o.poke; sb += o.strokeBegin; se += o.strokeEnd; }
    CHECK(sb == 1 && se == 1); }

  printf(fails ? "%d FAILED\n" : "all tests passed\n", fails);
  return fails ? 1 : 0;
}
