/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - 屏幕模块/    ST7789 显示驱动
 *   - (后续: EC11 编码器模块...)
 */

#include "屏幕模块/screen.h"

// ---------------------------------------------------------------------------
// 测试 1: 纯色填充 — 验证 RGB 通道和 SPI 通信
// ---------------------------------------------------------------------------
static void test_fill_colors() {
    // 上半屏: 红 / 绿 / 蓝
    tft.fillRect(0,   0, 80, 120, TFT_RED);
    tft.fillRect(80,  0, 80, 120, TFT_GREEN);
    tft.fillRect(160, 0, 80, 120, TFT_BLUE);

    // 下半屏: 白/黑/黄/青
    tft.fillRect(0,   120, 120, 60, TFT_WHITE);
    tft.fillRect(0,   180, 120, 60, TFT_BLACK);
    tft.fillRect(120, 120, 120, 60, TFT_YELLOW);
    tft.fillRect(120, 180, 120, 60, TFT_CYAN);
}

// ---------------------------------------------------------------------------
// 测试 2: 文字显示
// ---------------------------------------------------------------------------
static void test_text() {
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.drawString("ST7789", 10, 20, 4);

    tft.setTextColor(TFT_GREEN);
    tft.drawString("ESP32-S OK", 10, 70, 2);

    tft.setTextColor(TFT_YELLOW);
    tft.drawString("240x240 SPI", 10, 110, 1);
    tft.drawString("TFT_eSPI Driver", 10, 125, 1);

    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(" Highlight Text ", 10, 160, 2);

    tft.setTextColor(TFT_RED);
    tft.drawString("Red text", 10, 200, 2);
}

// ---------------------------------------------------------------------------
// 测试 3: 图形绘制
// ---------------------------------------------------------------------------
static void test_graphics() {
    tft.fillScreen(TFT_BLACK);

    tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
    tft.drawLine(0, 0, tft.width(), tft.height(), TFT_RED);
    tft.drawLine(tft.width(), 0, 0, tft.height(), TFT_BLUE);
    tft.drawRect(30, 30, 180, 180, TFT_GREEN);
    tft.drawRect(60, 60, 120, 120, TFT_YELLOW);
    tft.fillCircle(120, 120, 40, TFT_CYAN);
    tft.drawCircle(120, 120, 70, TFT_MAGENTA);
    tft.fillRect(85, 85, 70, 70, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("OK", 97, 108, 4);
}

// ---------------------------------------------------------------------------
// 测试 4: 弹跳方块动画
// ---------------------------------------------------------------------------
static void test_animation() {
    tft.fillScreen(TFT_BLACK);

    static const uint16_t colors[] = {
        TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW,
        TFT_CYAN, TFT_MAGENTA, TFT_ORANGE, TFT_PINK
    };

    int size = 30, x = 0, y = 0, dx = 3, dy = 2, ci = 0;

    for (int frame = 0; frame < 300; frame++) {
        tft.fillRect(x, y, size, size, TFT_BLACK);
        x += dx;
        y += dy;
        if (x <= 0 || x + size >= tft.width())  { dx = -dx; ci = (ci + 1) % 8; }
        if (y <= 0 || y + size >= tft.height()) { dy = -dy; ci = (ci + 1) % 8; }
        tft.fillRect(x, y, size, size, colors[ci]);
        delay(10);
    }
}

// ===========================================================================
// setup & loop
// ===========================================================================

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - ST7789 屏幕测试");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    Serial.println("Test 1/4: Fill colors");
    test_fill_colors();
    delay(2000);

    Serial.println("Test 2/4: Text");
    test_text();
    delay(2000);

    Serial.println("Test 3/4: Graphics");
    test_graphics();
    delay(2000);

    Serial.println("Test 4/4: Animation");
    test_animation();

    Serial.println("=== All tests passed, looping... ===\n");
    delay(1000);
}
