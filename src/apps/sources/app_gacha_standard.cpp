#include "hal/hal.h"
#include "lang/ui_strings.h"
#include "sys/app_base.h"
#include "sys/app_manager.h"
#include "sys/sys_gacha.h"
#include "ui/ui_theme.h"

#include <stdio.h>

namespace {

enum class GachaPhase : uint8_t {
    Idle,
    Animating,
    Result,
};

struct ScanLayout {
    int box_size;
    int gap_x;
    int total_w;
    int start_x;
    int start_y;
    int scan_top;
    int scan_h;
};

int clampInt(int value, int min_value, int max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

uint16_t colorForRarity(SysGacha::Rarity rarity)
{
    switch (rarity) {
        case SysGacha::Rarity::Star6:
            return UITheme::Gacha::kStar6;
        case SysGacha::Rarity::Star5:
            return UITheme::Gacha::kStar5;
        case SysGacha::Rarity::Star4:
            return UITheme::Gacha::kStar4;
        case SysGacha::Rarity::Star3:
        default:
            return UITheme::Gacha::kStar3;
    }
}

ScanLayout calcScanLayout(int sw, int sh)
{
    ScanLayout layout = {};
    int side_margin = 16;
    layout.gap_x = 4;
    layout.box_size = (sw - side_margin * 2 - layout.gap_x * (UITheme::Gacha::kPullCount - 1)) / UITheme::Gacha::kPullCount;
    layout.box_size = clampInt(layout.box_size, 12, 22);
    layout.total_w = layout.box_size * UITheme::Gacha::kPullCount + layout.gap_x * (UITheme::Gacha::kPullCount - 1);
    layout.start_x = (sw - layout.total_w) / 2;
    layout.start_y = sh / 2 - layout.box_size / 2 + 8;
    if (layout.start_y + layout.box_size > sh - 4) {
        layout.start_y = sh - layout.box_size - 4;
    }
    layout.scan_top = layout.start_y - 5;
    layout.scan_h = layout.box_size + 10;
    return layout;
}

class AppGachaStandard : public AppBase {
public:
    void onCreate() override
    {
        phase_ = GachaPhase::Idle;
        scroll_offset_ = 0;
        dirty_ = true;
    }

    void onResume() override
    {
        dirty_ = true;
    }

    void onLoop() override
    {
        updateAnimation();

        if (!dirty_) {
            return;
        }

        dirty_ = false;
        draw();
    }

    void onDestroy() override {}

    void onKnob(int delta) override
    {
        if (phase_ != GachaPhase::Result) {
            return;
        }

        int max_offset = UITheme::Gacha::kPullCount - UITheme::Gacha::kResultVisibleRows;
        scroll_offset_ = clampInt(scroll_offset_ + delta, 0, max_offset);
        dirty_ = true;
    }

    void onKeyShort() override
    {
        if (phase_ == GachaPhase::Idle) {
            executeTenPull();
            phase_ = GachaPhase::Animating;
            anim_start_ms_ = millis();
            last_frame_ms_ = 0;
            dirty_ = true;
            return;
        }

        if (phase_ == GachaPhase::Animating) {
            revealAll();
            phase_ = GachaPhase::Result;
            scroll_offset_ = 0;
            dirty_ = true;
            return;
        }

        phase_ = GachaPhase::Idle;
        scroll_offset_ = 0;
        dirty_ = true;
    }

    void onKeyLong() override {}

private:
    GachaPhase phase_ = GachaPhase::Idle;
    SysGacha::PullResult pulls_[UITheme::Gacha::kPullCount] = {};
    bool revealed_[UITheme::Gacha::kPullCount] = {};
    int scroll_offset_ = 0;
    uint32_t anim_start_ms_ = 0;
    uint32_t last_frame_ms_ = 0;
    bool dirty_ = true;

    void executeTenPull()
    {
        SysGacha::ExecuteTenPull(pulls_, UITheme::Gacha::kPullCount);
        for (int i = 0; i < UITheme::Gacha::kPullCount; ++i) {
            revealed_[i] = false;
        }
        scroll_offset_ = 0;
    }

    void revealAll()
    {
        for (int i = 0; i < UITheme::Gacha::kPullCount; ++i) {
            revealed_[i] = true;
        }
    }

    void updateAnimation()
    {
        if (phase_ != GachaPhase::Animating) {
            return;
        }

        uint32_t now = millis();
        uint32_t elapsed = now - anim_start_ms_;

        if (elapsed >= UITheme::Gacha::kAnimDurationMs + 120) {
            revealAll();
            phase_ = GachaPhase::Result;
            scroll_offset_ = 0;
            dirty_ = true;
            return;
        }

        ScanLayout layout = calcScanLayout(HAL_Get_Screen_Width(), HAL_Get_Screen_Height());
        for (int i = 0; i < UITheme::Gacha::kPullCount; ++i) {
            int center_x = i * (layout.box_size + layout.gap_x) + layout.box_size / 2;
            uint32_t trigger_ms = (uint32_t)center_x * UITheme::Gacha::kAnimDurationMs / layout.total_w;
            if (elapsed >= trigger_ms && !revealed_[i]) {
                revealed_[i] = true;
                dirty_ = true;
            }
        }

        if (last_frame_ms_ == 0 || now - last_frame_ms_ >= UITheme::Gacha::kAnimFrameMs) {
            last_frame_ms_ = now;
            dirty_ = true;
        }
    }

    void draw()
    {
        HAL_BeginFrame(UITheme::Color::kBackground);

        if (phase_ == GachaPhase::Idle) {
            drawIdle();
        } else if (phase_ == GachaPhase::Animating) {
            drawAnimating();
        } else {
            drawResult();
        }

        HAL_Flush();
    }

    void drawCenteredText(int y, const char *text, const uint8_t *font, uint16_t color)
    {
        HAL_SetFont(font);
        HAL_SetTextColor(color, UITheme::Color::kBackground);
        int x = (HAL_Get_Screen_Width() - HAL_Get_Text_Width(text)) / 2;
        if (x < 0) x = 0;
        HAL_DrawText(x, y, text);
    }

    void drawIdle()
    {
        const SysGacha::Stats &stats = SysGacha::GetStats();
        const SysGacha::PityState &pity = SysGacha::GetPityState();

        drawCenteredText(8, UIStrings::GachaStandardTitle(), u8g2_font_wqy16_t_gb2312, UITheme::Color::kText);
        drawCenteredText(36, UIStrings::GachaStartHint(), u8g2_font_wqy12_t_gb2312, UITheme::Color::kAccent);

        char buf[48];
        snprintf(buf, sizeof(buf), "总抽 %lu  距6星 %u", (unsigned long)stats.total, pity.pulls_since_star6);
        drawCenteredText(56, buf, u8g2_font_wqy12_t_gb2312, UITheme::Gacha::kDim);
    }

    void drawAnimating()
    {
        int sw = HAL_Get_Screen_Width();
        ScanLayout layout = calcScanLayout(sw, HAL_Get_Screen_Height());

        drawCenteredText(6, UIStrings::GachaAnimatingText(), u8g2_font_wqy12_t_gb2312, UITheme::Color::kText);

        for (int i = 0; i < UITheme::Gacha::kPullCount; ++i) {
            int x = layout.start_x + i * (layout.box_size + layout.gap_x);
            int y = layout.start_y;
            if (revealed_[i]) {
                HAL_FillRect(x, y, layout.box_size, layout.box_size, colorForRarity(pulls_[i].rarity));
                HAL_DrawRect(x, y, layout.box_size, layout.box_size, UITheme::Gacha::kStar3);
            } else {
                HAL_DrawRect(x, y, layout.box_size, layout.box_size, UITheme::Gacha::kDim);
            }
        }

        uint32_t elapsed = millis() - anim_start_ms_;
        if (elapsed < UITheme::Gacha::kAnimDurationMs) {
            int scan_x = layout.start_x + (int)((uint32_t)layout.total_w * elapsed / UITheme::Gacha::kAnimDurationMs);
            scan_x = clampInt(scan_x, layout.start_x, layout.start_x + layout.total_w - 1);
            HAL_DrawFastVLine(scan_x, layout.scan_top, layout.scan_h, UITheme::Color::kPrimary);
            if (scan_x + 1 < sw) {
                HAL_DrawFastVLine(scan_x + 1, layout.scan_top + 2, layout.scan_h - 4, UITheme::Color::kAccent);
            }
        }
    }

    void drawResult()
    {
        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(8, 2, UIStrings::GachaResultTitle());

        const int row_h = 12;
        const int top_y = 15;
        char line[64];
        for (int row = 0; row < UITheme::Gacha::kResultVisibleRows; ++row) {
            int idx = scroll_offset_ + row;
            if (idx >= UITheme::Gacha::kPullCount) {
                break;
            }

            const char *name = pulls_[idx].op ? pulls_[idx].op->name : "未知";
            snprintf(line, sizeof(line), "%02d  %d★  %s", idx + 1, SysGacha::RarityStars(pulls_[idx].rarity), name);
            HAL_SetTextColor(colorForRarity(pulls_[idx].rarity), UITheme::Color::kBackground);
            HAL_DrawText(8, top_y + row * row_h, line);
        }

        drawScrollBar();
    }

    void drawScrollBar()
    {
        const int max_offset = UITheme::Gacha::kPullCount - UITheme::Gacha::kResultVisibleRows;
        if (max_offset <= 0) {
            return;
        }

        int sw = HAL_Get_Screen_Width();
        int sh = HAL_Get_Screen_Height();
        int track_top = 14;
        int track_h = sh - track_top - 4;
        int bar_h = track_h * UITheme::Gacha::kResultVisibleRows / UITheme::Gacha::kPullCount;
        if (bar_h < 6) bar_h = 6;
        int bar_y = track_top + (track_h - bar_h) * scroll_offset_ / max_offset;
        HAL_FillRect(sw - 6, bar_y, 3, bar_h, UITheme::Color::kPrimary);
    }
};

} // namespace

AppGachaStandard g_app_gacha_standard;
AppBase *appGachaStandard = &g_app_gacha_standard;
