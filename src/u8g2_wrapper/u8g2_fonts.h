// ============================================================
// U8g2_for_TFT_eSPI 字库桥接 (兼容 U8g2 v2.35+)
//
// U8g2 v2.35+ 把字体声明放到了 clib/u8g2.h，但其结构体和
// wrapper 的 16 位颜色定义不兼容。本文件独立提供所有缺失的
// 类型/宏/字体声明，不引入 clib/u8g2.h。
// ============================================================

#ifndef U8G2_FONT_BRIDGE_H
#define U8G2_FONT_BRIDGE_H

#include <stdint.h>

// ---- 类型定义 (wrapper 内部使用) ----
typedef uint16_t u8g2_uint_t;

// ---- 兼容宏 (来自 u8x8.h, 避免包含整个头) ----
#ifndef U8X8_NOINLINE
  #define U8X8_NOINLINE __attribute__((noinline))
#endif

#ifndef u8x8_pgm_read
  #ifdef ESP32
    #define u8x8_pgm_read(adr) (*(const uint8_t *)(adr))
  #else
    #define u8x8_pgm_read(adr) (*(const uint8_t *)(adr))
  #endif
#endif

// ---- 字体 SECTION 宏 ----
#ifndef U8G2_FONT_SECTION
  #define U8G2_FONT_SECTION(name) /* */
#endif

// ---- 常用字库 extern 声明 ----

// 英文
extern const uint8_t u8g2_font_unifont_t_symbols[]
  U8G2_FONT_SECTION("u8g2_font_unifont_t_symbols");

// 中文 (文泉驿 12-16px)
extern const uint8_t u8g2_font_wqy12_t_gb2312[]
  U8G2_FONT_SECTION("u8g2_font_wqy12_t_gb2312");
extern const uint8_t u8g2_font_wqy13_t_gb2312[]
  U8G2_FONT_SECTION("u8g2_font_wqy13_t_gb2312");
extern const uint8_t u8g2_font_wqy14_t_gb2312[]
  U8G2_FONT_SECTION("u8g2_font_wqy14_t_gb2312");
extern const uint8_t u8g2_font_wqy15_t_gb2312[]
  U8G2_FONT_SECTION("u8g2_font_wqy15_t_gb2312");
extern const uint8_t u8g2_font_wqy16_t_gb2312[]
  U8G2_FONT_SECTION("u8g2_font_wqy16_t_gb2312");

#endif
