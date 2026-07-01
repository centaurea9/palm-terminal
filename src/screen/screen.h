/**
 * ST7789 显示屏驱动模块
 *
 * 引脚: SCK=18, MOSI=23, DC=4, RST=16, CS=17, BL=32
 * 英文: TFT_eSPI drawString()
 * 中文: U8g2_for_TFT_eSPI (文泉驿字体)
 */

#ifndef SCREEN_H
#define SCREEN_H

#include <TFT_eSPI.h>
#include <U8g2_for_TFT_eSPI.h>

// 全局实例
extern TFT_eSPI tft;
extern U8g2_for_TFT_eSPI u8g2;

//设置字体大小（12-16）
// u8g2.setFont(u8g2_font_wqy12_t_gb2312);

// 初始化屏幕 (含 U8g2 绑定)
void screen_init();

// 背光控制
void screen_backlight(bool on);
void screen_brightness(uint8_t level);

// 中文/英文混排文字 (UTF-8)
// 在 (x, y) 处打印文字，自动解码 UTF-8
void screen_draw_text(int x, int y, const char *text);

// 设置文字透明度: 0=不透明(有背景色), 1=透明
void screen_text_transparent(bool on);

// 设置 U8g2 文字颜色
void screen_text_color(uint16_t fg, uint16_t bg);

#endif
