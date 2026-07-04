#include "hal/hal.h"
#include "lang/ui_strings.h"
#include "sys/app_base.h"
#include "sys/app_manager.h"
#include "sys/sys_gacha.h"
#include "ui/ui_theme.h"

#include <stdio.h>

namespace {

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

class AppGachaHistory : public AppBase {
public:
    void onCreate() override
    {
        scroll_offset_ = 0;
        dirty_ = true;
    }

    void onResume() override
    {
        dirty_ = true;
    }

    void onLoop() override
    {
        if (!dirty_) {
            return;
        }

        dirty_ = false;
        draw();
    }

    void onDestroy() override {}

    void onKnob(int delta) override
    {
        scroll_offset_ = clampInt(scroll_offset_ + delta, 0, maxScrollOffset());
        dirty_ = true;
    }

    void onKeyShort() override
    {
        appManager.pop();
    }

    void onKeyLong() override {}

private:
    int scroll_offset_ = 0;
    bool dirty_ = true;

    int lineCount() const
    {
        return 7 + SysGacha::GetLastPullCount();
    }

    int maxScrollOffset() const
    {
        int max_offset = lineCount() - UITheme::Gacha::kResultVisibleRows;
        return max_offset > 0 ? max_offset : 0;
    }

    void draw()
    {
        HAL_BeginFrame(UITheme::Color::kBackground);

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(8, 2, UIStrings::GachaHistoryTitle());

        for (int row = 0; row < UITheme::Gacha::kResultVisibleRows; ++row) {
            int line_index = scroll_offset_ + row;
            drawLine(line_index, 15 + row * 12);
        }

        drawScrollBar();
        HAL_Flush();
    }

    void drawLine(int line_index, int y)
    {
        const SysGacha::Stats &stats = SysGacha::GetStats();
        const SysGacha::PityState &pity = SysGacha::GetPityState();
        const SysGacha::PullResult *last = SysGacha::GetLastPulls();
        char line[64];
        uint16_t color = UITheme::Gacha::kStar3;

        switch (line_index) {
            case 0:
                snprintf(line, sizeof(line), "总抽数: %lu", (unsigned long)stats.total);
                color = UITheme::Color::kText;
                break;
            case 1:
                snprintf(line, sizeof(line), "6星: %lu", (unsigned long)stats.star6);
                color = UITheme::Gacha::kStar6;
                break;
            case 2:
                snprintf(line, sizeof(line), "5星: %lu", (unsigned long)stats.star5);
                color = UITheme::Gacha::kStar5;
                break;
            case 3:
                snprintf(line, sizeof(line), "4星: %lu", (unsigned long)stats.star4);
                color = UITheme::Gacha::kStar4;
                break;
            case 4:
                snprintf(line, sizeof(line), "3星: %lu", (unsigned long)stats.star3);
                color = UITheme::Gacha::kStar3;
                break;
            case 5:
                snprintf(line, sizeof(line), "距6星: %u", pity.pulls_since_star6);
                color = UITheme::Gacha::kDim;
                break;
            case 6:
                snprintf(line, sizeof(line), "本期抽数: %u", pity.banner_pulls);
                color = UITheme::Gacha::kDim;
                break;
            default: {
                int pull_index = line_index - 7;
                if (pull_index < 0 || pull_index >= SysGacha::GetLastPullCount()) {
                    return;
                }
                const char *name = last[pull_index].op ? last[pull_index].op->name : "未知";
                snprintf(line, sizeof(line), "%02d %d★ %s", pull_index + 1, SysGacha::RarityStars(last[pull_index].rarity), name);
                color = colorForRarity(last[pull_index].rarity);
                break;
            }
        }

        HAL_SetTextColor(color, UITheme::Color::kBackground);
        HAL_DrawText(8, y, line);
    }

    void drawScrollBar()
    {
        int max_offset = maxScrollOffset();
        if (max_offset <= 0) {
            return;
        }

        int sw = HAL_Get_Screen_Width();
        int sh = HAL_Get_Screen_Height();
        int track_top = 14;
        int track_h = sh - track_top - 4;
        int total = lineCount();
        int bar_h = track_h * UITheme::Gacha::kResultVisibleRows / total;
        if (bar_h < 6) bar_h = 6;
        int bar_y = track_top + (track_h - bar_h) * scroll_offset_ / max_offset;
        HAL_FillRect(sw - 6, bar_y, 3, bar_h, UITheme::Color::kPrimary);
    }
};

} // namespace

AppGachaHistory g_app_gacha_history;
AppBase *appGachaHistory = &g_app_gacha_history;
