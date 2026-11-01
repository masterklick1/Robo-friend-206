// Pin map for Waveshare ESP32-S3-Touch-AMOLED-2.06.
// Values taken from the manufacturer's official pin_config.h
// (waveshareteam/ESP32-S3-Touch-AMOLED-2.06, examples/arduino/libraries/board_config).
#pragma once

// Display (CO5300, QSPI)
#define LCD_SDIO0 4
#define LCD_SDIO1 5
#define LCD_SDIO2 6
#define LCD_SDIO3 7
#define LCD_SCLK 11
#define LCD_CS 12
#define LCD_RESET 8
#define LCD_WIDTH 410
#define LCD_HEIGHT 502

// Touch (FT3168, I2C) - shared I2C bus with PMU / IMU / RTC / audio codecs
#define IIC_SDA 15
#define IIC_SCL 14
#define TP_INT 38     // unused: we poll
#define TP_RESET 9    // unused: official example does not reset the chip either
