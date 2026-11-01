// Renders every mood to a PPM frame (full 410x502 screen) so the eyes can be
// previewed on a PC without hardware.
//   g++ -O2 -std=c++17 -I../firmware/robo_friend render_preview.cpp ../firmware/robo_friend/eyes.cpp ../firmware/robo_friend/mood_engine.cpp -o render_preview
//   ./render_preview outdir   -> outdir/<mood>.ppm
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <string>
#include "eyes.h"
using namespace rf;

int main(int argc, char** argv) {
  std::string out = argc > 1 ? argv[1] : ".";
  const int W = Eyes::kScreenW, H = 502;
  std::vector<uint16_t> band(Eyes::kPixels);
  Eyes eyes;
  for (int m = 0; m < (int)Mood::COUNT; m++) {
    Mood mood = (Mood)m;
    eyes.snapToMood(mood);
    eyes.setBlink(1.0f);
    eyes.setGaze(0, 0);
    eyes.update(0);
    eyes.render(band.data());
    std::vector<uint8_t> rgb(W * H * 3, 0);
    for (int y = 0; y < Eyes::kBandH; y++)
      for (int x = 0; x < W; x++) {
        uint16_t p = band[y * W + x];
        uint8_t* d = &rgb[((y + Eyes::kBandY0) * W + x) * 3];
        d[0] = ((p >> 11) & 31) * 255 / 31; d[1] = ((p >> 5) & 63) * 255 / 63; d[2] = (p & 31) * 255 / 31;
      }
    std::string path = out + "/" + moodName(mood) + ".ppm";
    FILE* f = fopen(path.c_str(), "wb");
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    fwrite(rgb.data(), 1, rgb.size(), f);
    fclose(f);
  }
  // timing: render cost of the busiest mood (neutral, full eyes)
  eyes.snapToMood(Mood::Neutral);
  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < 200; i++) { eyes.update(i * 33); eyes.render(band.data()); }
  auto t1 = std::chrono::steady_clock::now();
  printf("host render: %.3f ms/frame\n", std::chrono::duration<double, std::milli>(t1 - t0).count() / 200);
  return 0;
}
