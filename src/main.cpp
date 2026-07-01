/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    TFT_eSPI 显示驱动 (ST7789, 76×284 竖屏)
 */

#include "screen/screen.h"

// ===========================================================================
// setup & loop
// ===========================================================================

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - 76x284 ST7789 竖屏");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    int w = tft.width();   // 76
    int h = tft.height();  // 284

    // 三等分高度: 红 / 黄 / 蓝 横条
    int seg = h / 3;  // 94

    tft.fillRect(0, 0,         w, seg,          TFT_RED);
    tft.fillRect(0, seg,       w, seg,          TFT_YELLOW);
    tft.fillRect(0, seg * 2,   w, h - seg * 2,  TFT_BLUE);

    delay(5000);

    tft.fillRect(0, 0,         w, seg,          TFT_BLUE);
    tft.fillRect(0, seg,       w, seg,          TFT_YELLOW);
    tft.fillRect(0, seg * 2,   w, h - seg * 2,  TFT_RED);

    delay(5000);
}
