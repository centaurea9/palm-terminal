/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    TFT_eSPI 显示驱动 (ST7789, 284×76 横屏)
 */

#include "screen/screen.h"

// ===========================================================================
// setup & loop
// ===========================================================================

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - 284x76 ST7789 横屏");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    int w = tft.width();   // 284
    int h = tft.height();  // 76

    // 三等分高度: 红 / 黄 / 蓝 横条
    int h1 = h / 3;          // 25
    int h2 = h / 3;          // 25
    int h3 = h - h1 - h2;    // 26

    tft.fillRect(0, 0,        w, h1, TFT_RED);
    tft.fillRect(0, h1,       w, h2, TFT_YELLOW);
    tft.fillRect(0, h1 + h2,  w, h3, TFT_BLUE);

    delay(5000);

    tft.fillRect(0, 0,        w, h1, TFT_BLUE);
    tft.fillRect(0, h1,       w, h2, TFT_YELLOW);
    tft.fillRect(0, h1 + h2,  w, h3, TFT_RED);

    delay(5000);
}
