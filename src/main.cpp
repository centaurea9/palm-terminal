/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    显示驱动 (ST7789, 76×284 竖屏)
 *   - screen/    中英文文字渲染 (U8g2_for_TFT_eSPI)
 */

#include "screen/screen.h"

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - 76x284 ST7789");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    tft.fillScreen(TFT_BLACK);
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    // ---------- 顶部居中: 干员寻访 ----------
    
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(14, 2, "星期天");

    // ---------- 右下角竖排: 博士终端 (端字在最底部) ----------
    //

   
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(60,61, "博士终端");

    delay(10000);
}
