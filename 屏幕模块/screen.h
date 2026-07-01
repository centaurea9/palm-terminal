/**
 * ST7789 显示屏驱动模块
 *
 * 引脚: SCK=18, MOSI=23, DC=4, RST=16, CS=17, BL=32
 * 配置见 ../platformio.ini build_flags
 */

#ifndef SCREEN_H
#define SCREEN_H

#include <TFT_eSPI.h>

// 全局屏幕实例
extern TFT_eSPI tft;

// 初始化屏幕
void screen_init();

// 打开/关闭背光
void screen_backlight(bool on);

// 设置背光亮度 (0~255)
void screen_brightness(uint8_t level);

#endif
