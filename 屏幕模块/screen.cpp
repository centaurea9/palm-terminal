#include "screen.h"

TFT_eSPI tft = TFT_eSPI();

void screen_init() {
    tft.init();
    tft.setRotation(0);       // 0/1/2/3, 按实际屏幕方向调整
    tft.fillScreen(TFT_BLACK);
}

void screen_backlight(bool on) {
    if (on)
        digitalWrite(TFT_BL, HIGH);
    else
        digitalWrite(TFT_BL, LOW);
}

void screen_brightness(uint8_t level) {
    // 使用 LEDC PWM 实现无级调光, 频道 0, 频率 5kHz, 8位分辨率
    static bool configured = false;
    if (!configured) {
        ledcSetup(0, 5000, 8);
        ledcAttachPin(TFT_BL, 0);
        configured = true;
    }
    ledcWrite(0, level);
}
