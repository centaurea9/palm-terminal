#ifndef HAL_HAL_H
#define HAL_HAL_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "u8g2_wrapper/u8g2_fonts.h"

void HAL_Init();
void HAL_PollInput();

int HAL_Get_Knob_Delta();
bool HAL_Consume_Click();
bool HAL_Consume_LongPress();

int HAL_Get_Screen_Width();
int HAL_Get_Screen_Height();

void HAL_BeginFrame(uint16_t color = TFT_BLACK);
void HAL_Flush();

void HAL_SetFont(const uint8_t *font);
int HAL_Get_Font_Ascent();
int HAL_Get_Font_Descent();
int HAL_Get_Text_Width(const char *text);

void HAL_SetTextColor(uint16_t fg, uint16_t bg);
void HAL_DrawText(int x, int y, const char *text);

void HAL_DrawRect(int x, int y, int w, int h, uint16_t color);
void HAL_FillRect(int x, int y, int w, int h, uint16_t color);
void HAL_FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
void HAL_DrawFastVLine(int x, int y, int h, uint16_t color);

uint16_t HAL_Color565(uint8_t r, uint8_t g, uint8_t b);

#endif
