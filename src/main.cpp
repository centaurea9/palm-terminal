/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    显示驱动 (ST7789, 284×76 横屏)
 *   - screen/    中英文文字渲染 (U8g2_for_TFT_eSPI)
 */

#include "screen/screen.h"

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - 284x76 ST7789");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    tft.fillScreen(TFT_BLACK);

    // ---------- 英文 (TFT_eSPI 原生) ----------
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    tft.drawString("EN: Hello Palm Terminal!", 5, 2);

    // ---------- 中文 (U8g2) ----------
    screen_text_color(TFT_WHITE, TFT_BLACK);
    screen_draw_text(5, 22, "你好，掌上终端！");

    // ---------- 中英文混排 ----------
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(5, 42, "ESP32 中文测试 OK");

    screen_text_color(TFT_YELLOW, TFT_BLACK);
    screen_draw_text(5, 62, "分辨率: 284x76 ST7789");

    delay(10000);

    // ---------- 第二页 ----------
    tft.fillScreen(TFT_BLACK);

    screen_text_color(TFT_RED, TFT_BLACK);
    screen_draw_text(5, 10, "温度: 26.5°C");

    screen_text_color(TFT_GREEN, TFT_BLACK);
    screen_draw_text(5, 30, "湿度: 58%");

    screen_text_color(TFT_WHITE, TFT_BLACK);
    screen_draw_text(5, 50, "状态: 正常运行中");

    screen_text_color(TFT_YELLOW, TFT_BLACK);
    screen_draw_text(5, 70, "V1.0 | 2026-07-01");

    delay(10000);
}
