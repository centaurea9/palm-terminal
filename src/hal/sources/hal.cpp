#include "hal/hal.h"

#include "input/ec11.h"
#include "screen/screen.h"
#include "sys/sys_constants.h"

namespace {

EC11Input g_encoder;

} // namespace

void HAL_Init()
{
    screen_init();

    EC11Config encoder_config;
    encoder_config.pin_a = SysConst::kEc11PinA;
    encoder_config.pin_b = SysConst::kEc11PinB;
    encoder_config.pin_button = SysConst::kEc11PinSw;
    encoder_config.long_press_ms = SysConst::kEc11LongPressMs;
    g_encoder.begin(encoder_config);
}

void HAL_PollInput()
{
    g_encoder.update();
}

int HAL_Get_Knob_Delta()
{
    return g_encoder.consume_steps();
}

bool HAL_Consume_Click()
{
    return g_encoder.consume_click();
}

bool HAL_Consume_LongPress()
{
    return g_encoder.consume_long_press();
}

int HAL_Get_Screen_Width()
{
    return tft.width();
}

int HAL_Get_Screen_Height()
{
    return tft.height();
}

void HAL_BeginFrame(uint16_t color)
{
    screen_sprite.fillSprite(color);
}

void HAL_Flush()
{
    screen_flush();
}

void HAL_SetFont(const uint8_t *font)
{
    u8g2.setFont(font);
}

int HAL_Get_Font_Ascent()
{
    return u8g2.getFontAscent();
}

int HAL_Get_Font_Descent()
{
    return u8g2.getFontDescent();
}

int HAL_Get_Text_Width(const char *text)
{
    return u8g2.getUTF8Width(text);
}

void HAL_SetTextColor(uint16_t fg, uint16_t bg)
{
    screen_text_color(fg, bg);
}

void HAL_DrawText(int x, int y, const char *text)
{
    screen_draw_text(x, y, text);
}

void HAL_DrawRect(int x, int y, int w, int h, uint16_t color)
{
    screen_sprite.drawRect(x, y, w, h, color);
}

void HAL_FillRect(int x, int y, int w, int h, uint16_t color)
{
    screen_sprite.fillRect(x, y, w, h, color);
}

void HAL_FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color)
{
    screen_sprite.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void HAL_DrawFastVLine(int x, int y, int h, uint16_t color)
{
    screen_sprite.drawFastVLine(x, y, h, color);
}

uint16_t HAL_Color565(uint8_t r, uint8_t g, uint8_t b)
{
    return tft.color565(r, g, b);
}
