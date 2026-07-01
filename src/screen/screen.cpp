#include "screen.h"

TFT_eSPI tft = TFT_eSPI();
U8g2_for_TFT_eSPI u8g2;

void screen_init() {
    tft.init();
    tft.setRotation(1);       // 竖屏 76×284
    tft.fillScreen(TFT_BLACK);

    // U8g2 绑定 TFT
    u8g2.begin(tft);
    u8g2.setFontMode(1);      // 透明模式
    u8g2.setFontDirection(0); // 从左到右
    u8g2.setForegroundColor(TFT_WHITE);
    u8g2.setBackgroundColor(TFT_BLACK);

    // 中文字体 (文泉驿 12px, GB2312)
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
}

void screen_backlight(bool on) {
    digitalWrite(TFT_BL, on ? HIGH : LOW);
}

void screen_brightness(uint8_t level) {
    static bool configured = false;
    if (!configured) {
        ledcSetup(0, 5000, 8);
        ledcAttachPin(TFT_BL, 0);
        configured = true;
    }
    ledcWrite(0, level);
}

void screen_draw_text(int x, int y, const char *text) {
    // y = 文字顶部坐标；U8g2 光标以基线为基准，加上 ascent 修正
    u8g2.setCursor(x, y + u8g2.getFontAscent());
    u8g2.print(text);
}

void screen_text_transparent(bool on) {
    u8g2.setFontMode(on ? 1 : 0);
}

void screen_text_color(uint16_t fg, uint16_t bg) {
    u8g2.setForegroundColor(fg);
    u8g2.setBackgroundColor(bg);
}
