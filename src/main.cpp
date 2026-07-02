/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    显示驱动 (ST7789, 76×284 横屏)
 *   - screen/    中英文文字渲染 (U8g2_for_TFT_eSPI)
 */

#include "screen/screen.h"
#include "images/ak.h"

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
   

    // 显示图片 164x76, 1-bit 位图
    // tft.fillScreen(TFT_WHITE);
   // tft.drawBitmap(60, 0, epd_bitmap_, 164, 76, TFT_WHITE,TFT_BLACK );
   // delay(2000);
    tft.fillScreen(TFT_BLACK);

    
    //长条屏幕x=90px左侧
     //显示星期
     u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);
     screen_draw_text(22, 15, "星期天");
   
     //显示时间
     u8g2.setFont(u8g2_font_freedoomr25_tn);
     screen_text_color(TFT_CYAN, TFT_BLACK);
     screen_draw_text(-3, 40, "10:16");
   
     // ---------- 竖线 ----------
     tft.drawFastVLine(89, 0, 76, TFT_CYAN);
     screen_text_color(TFT_CYAN, TFT_BLACK);
    
    // //x=90px右侧
     u8g2.setFont(u8g2_font_wqy12_t_gb2312);
     screen_draw_text(185,7, "天气预报");
     screen_draw_text(175,31, "干员寻访");
     screen_draw_text(185,56, "系统设置");
    
     // ---------- 斜线 ----------
     //tft.drawLine(100, 60, 180, 70, TFT_RED);
    // ---------- 矩形 ----------
     tft.drawRect(94, 24, 186, 22, TFT_YELLOW);

    delay(10000);
}
