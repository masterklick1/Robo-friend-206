# Robo Friend (v0.1)

A little robot friend that lives on your **Waveshare ESP32-S3-Touch-AMOLED-2.06**:
two big glowing eyes that blink, look around, follow your finger and change mood.

![moods](docs/preview.png)

*(rendered on a PC by `tools/render_preview.cpp` - the same drawing code runs on the board)*

## What it does in v0.1

| You do | It does |
| --- | --- |
| Tap it | Gets startled, then smiles |
| Tap it again and again (5x in 3 s) | Gets annoyed |
| Hold your finger on it or stroke it | Heart eyes while you pet it, happy afterwards |
| Touch anywhere | Eyes follow your finger |
| Leave it alone | Curious (25 s) -> bored (90 s) -> sleepy (3 min) -> falls asleep (4 min), screen dims |
| Touch it while asleep | Wakes up surprised |
| Ignore it for a long time | Gets sad |

Moods: neutral, happy, sad, angry, sleepy, sleeping, surprised, curious, love, bored.

Everything runs offline on the board. No cloud, no account.

## Hardware

Waveshare ESP32-S3-Touch-AMOLED-2.06 (410x502 CO5300 QSPI AMOLED, FT3168 touch).
Pin numbers come from the manufacturer's official `pin_config.h`.

## Build

### Option A - let GitHub build it (no local setup)
Push to GitHub. The **build-firmware** workflow runs the unit tests, compiles the sketch
and uploads the `.bin` files as an artifact (Actions -> latest run -> `robo-friend-firmware`).
Use the file ending in `.merged.bin`.

### Option B - Arduino IDE
1. Install the **esp32** board package by Espressif, version **3.3.11**.
2. Get the display library with CO5300 support: copy the folder
   `examples/arduino/libraries/Arduino_GFX` from
   <https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06> into your Arduino `libraries` folder.
3. Open `firmware/robo_friend/robo_friend.ino`.
4. Settings: Board **ESP32S3 Dev Module**, Flash Size **16MB**, Partition Scheme
   **16M Flash (3MB APP/9.9MB FATFS)**, PSRAM **OPI PSRAM**, USB CDC On Boot **Enabled**.
5. Upload.

If the board is not detected: hold **BOOT**, press **RESET**, release **BOOT**, then upload.

## Flash a prebuilt .merged.bin

```
python -m pip install esptool
esptool.py --chip esp32s3 write_flash 0x0 robo_friend.ino.merged.bin
```

## Develop without the board

```
# unit tests for the mood logic and touch gestures
cd test && g++ -std=c++17 -I../firmware/robo_friend test_logic.cpp \
  ../firmware/robo_friend/mood_engine.cpp ../firmware/robo_friend/gesture.cpp -o test_logic && ./test_logic

# render every mood to .ppm images
cd tools && g++ -O2 -std=c++17 -I../firmware/robo_friend render_preview.cpp \
  ../firmware/robo_friend/eyes.cpp ../firmware/robo_friend/mood_engine.cpp -o render_preview && ./render_preview ../docs
```

## Code layout

```
firmware/robo_friend/
  robo_friend.ino   glue: display, touch, main loop
  mood_engine.*     moods, reactions, sleep (pure C++, unit-tested)
  gesture.*         tap / hold / drag detection (pure C++, unit-tested)
  eyes.*            software eye renderer, RGB565 (pure C++, previewable on a PC)
  touch_ft3168.h    tiny own touch driver
  board_pins.h      pin map
```

The display is updated as a single horizontal strip around the eyes, not the whole screen,
so the animation stays smooth.

## Roadmap

- v0.2: shake / pick-up / steps (IMU), time-of-day sleep (RTC), battery mood
- v0.3: weather mood (Wi-Fi), Wi-Fi setup from a phone, sounds
- v0.4: mini-games, clock mode, over-the-air updates, browser flasher
- later: optional AI chat with your own API key

## Licensing notes

- Do **not** use Waveshare's `Arduino_DriveBus` library in this project: it is GPL-3.0.
  This repo uses its own small touch driver instead.
- The display library (Arduino_GFX) is a third-party dependency - keep its license notice
  when you distribute builds. See `NOTICE.md`.
- Choose a license for this repository before you publish it.

## Status

Written without access to the physical board. The mood logic and the eye renderer are tested
on a PC; display and touch code follows the manufacturer's official examples but still needs
a first run on real hardware.
