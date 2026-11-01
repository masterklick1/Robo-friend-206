# Robo Friend - animated eyes firmware for Waveshare ESP32-S3-Touch-AMOLED-2.06

English | [Русский](#русский)

![moods](docs/preview.png)

Two big glowing eyes for your Waveshare AMOLED board. They blink, look around, follow your finger and change mood. Runs fully on the board: no Wi-Fi, no account, no cloud.

> **Status: BETA (v0.1).** Not yet verified on every board. Testers wanted - open an Issue and tell us what you see.

## Compatible board

**Only: Waveshare ESP32-S3-Touch-AMOLED-2.06** (2.06" AMOLED, 410x502, CO5300 display, FT3168 touch).

**NOT compatible:** other Waveshare AMOLED boards (1.43", 1.64", 1.75", 1.8", 1.91", 2.16", 2.41"), plain ESP32 / ESP32-S2 / ESP32-C3 boards, or any board without this screen. Do not flash it on them.

## Install (no programming needed)

You need the board, a USB-C cable **that carries data**, and Chrome or Edge.

1. Download `robo_friend.ino.merged.bin` from the **Releases** page.
2. Plug the board into your computer.
3. Download mode: **hold BOOT, press and release RESET, release BOOT.** A blank screen is normal.
4. Open https://adafruit.github.io/Adafruit_WebSerial_ESPTool/ in Chrome or Edge.
5. Click **Connect**, choose the board's serial port.
6. Set offset **`0x0`**, click **Choose a file...**, select `robo_friend.ino.merged.bin`.
7. Click **Program**, wait about a minute, do not unplug.
8. Press **RESET**. The eyes should appear.

## Restore the original firmware

Flashing this does not damage the board. Take `ESP32-S3-Touch-AMOLED-2.06_FactoryOnly_....bin` from the `firmware` folder of
https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06 and flash it the same way at offset `0x0`.

## Problems

- Port not found: try another cable (many are charge-only) or USB port, repeat BOOT + RESET.
- Black screen: re-plug and flash again; if still black, open an Issue.
- Eyes appear but touch is wrong: open an Issue and describe it (include the exact board name).

---

# Русский

Два светящихся глаза для платы Waveshare с AMOLED-экраном: моргают, оглядываются, следят за пальцем, меняют настроение. Работает полностью на плате, без Wi-Fi и облака.

> **Статус: БЕТА (v0.1).** На всех платах ещё не проверено. Нужны тестировщики: создайте Issue и напишите, что получилось.

## Какая плата подходит

**Только: Waveshare ESP32-S3-Touch-AMOLED-2.06** (экран 2,06″, 410x502, CO5300, тач FT3168).

**Не подходит:** остальные платы Waveshare с AMOLED (1.43″, 1.64″, 1.75″, 1.8″, 1.91″, 2.16″, 2.41″), обычные ESP32 / ESP32-S2 / ESP32-C3 и любые платы без этого экрана. На них прошивать нельзя.

## Установка (программировать не нужно)

Нужны: плата, кабель USB-C **с передачей данных**, браузер Chrome или Edge.

1. Скачайте `robo_friend.ino.merged.bin` на странице **Releases**.
2. Подключите плату к компьютеру.
3. Режим загрузки: **зажмите BOOT, нажмите и отпустите RESET, отпустите BOOT.** Тёмный экран - это нормально.
4. Откройте в Chrome или Edge: https://adafruit.github.io/Adafruit_WebSerial_ESPTool/
5. Нажмите **Connect**, выберите порт платы.
6. Смещение **`0x0`**, **Choose a file...**, выберите `robo_friend.ino.merged.bin`.
7. Нажмите **Program**, подождите около минуты, не отключайте.
8. Нажмите **RESET**. Должны появиться глаза.

## Вернуть заводскую прошивку

Плату это не повреждает. Файл `ESP32-S3-Touch-AMOLED-2.06_FactoryOnly_....bin` лежит в папке `firmware` репозитория
https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06, прошейте его так же со смещением `0x0`.

## Если не работает

- Порт не виден: другой кабель (часто только для зарядки) или USB-порт, повторите BOOT + RESET.
- Чёрный экран: переподключите и прошейте снова; если не помогло, создайте Issue.
- Глаза есть, но тач работает неправильно: создайте Issue, укажите точное название платы.
