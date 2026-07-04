#ifndef UI_THEME_H
#define UI_THEME_H

#include <TFT_eSPI.h>
#include "sys/sys_constants.h"

namespace UITheme {

namespace Color {
constexpr uint16_t kBackground = TFT_BLACK;
constexpr uint16_t kPrimary = TFT_CYAN;
constexpr uint16_t kAccent = TFT_YELLOW;
constexpr uint16_t kText = TFT_CYAN;
} // namespace Color

namespace Menu {
constexpr float kVisualEps = SysConst::kMenuVisualEps;
constexpr uint16_t kFrameFastMs = SysConst::kFrameFastMs;
constexpr int kVisibleRadius = SysConst::kVisibleRadius;
constexpr int kDividerX = SysConst::kMenuDividerX;
constexpr float kCurveFactor = SysConst::kMenuCurveFactor;
constexpr float kCurveSpeedBoost = SysConst::kMenuCurveSpeedBoost;
constexpr float kSlingSpeedBoost = SysConst::kMenuSlingSpeedBoost;
constexpr int kCenterFlingX = SysConst::kMenuCenterFlingX;
constexpr int kCenterMinLeftGap = SysConst::kMenuCenterMinLeftGap;
constexpr int kBoxPadX = SysConst::kMenuBoxPadX;
constexpr int kScrollBarW = SysConst::kMenuScrollBarW;
constexpr int kBoxH = SysConst::kMenuBoxH;
constexpr int kItemSpacingY = SysConst::kMenuItemSpacingY;
constexpr int kStackMax = SysConst::kMenuStackMax;
} // namespace Menu

namespace Layout {
constexpr int kLeftTitleX = 8;
constexpr int kLeftTitleY = 4;
constexpr int kLeftFocusX = 8;
constexpr int kLeftFocusY = 18;
constexpr int kClockX = -3;
constexpr int kClockY = 40;
constexpr int kContentX = 100;
constexpr int kContentTitleY = 15;
constexpr int kContentBodyY = 45;
} // namespace Layout

} // namespace UITheme

#endif
