#ifndef SYS_CONSTANTS_H
#define SYS_CONSTANTS_H

#include <stdint.h>

namespace SysConst {

constexpr uint32_t kSerialBaud = 115200;

constexpr uint8_t kEc11PinA = 26;
constexpr uint8_t kEc11PinB = 27;
constexpr uint8_t kConfirmButtonPin = 0;
constexpr uint16_t kConfirmButtonLongPressMs = 700;

constexpr float kMenuVisualEps = 0.02f;
constexpr uint16_t kFrameFastMs = 16;
constexpr int kVisibleRadius = 3;
constexpr int kMenuDividerX = 89;
constexpr float kMenuCurveFactor = 7.0f;
constexpr float kMenuCurveSpeedBoost = 6.5f;
constexpr float kMenuSlingSpeedBoost = 4.0f;
constexpr int kMenuCenterFlingX = 12;
constexpr int kMenuCenterMinLeftGap = 28;
constexpr int kMenuBoxPadX = 4;
constexpr int kMenuScrollBarW = 4;
constexpr int kMenuBoxH = 22;
constexpr int kMenuItemSpacingY = 18;
constexpr int kMenuStackMax = 8;

} // namespace SysConst

#endif
