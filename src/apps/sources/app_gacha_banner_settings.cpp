#include "hal/hal.h"
#include "lang/ui_strings.h"
#include "sys/app_base.h"
#include "sys/app_manager.h"
#include "sys/sys_gacha.h"
#include "ui/ui_theme.h"

#include <stdio.h>

namespace {

enum class BannerSettingsPhase : uint8_t {
    SelectField,
    EditUpA,
    EditUpB,
};

int clampInt(int value, int min_value, int max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

uint8_t stepStar6Index(uint8_t index, int delta)
{
    int count = SysGacha::GetStar6PoolCount();
    if (count <= 0) {
        return 0;
    }

    int next = (int)index + delta;
    while (next < 0) {
        next += count;
    }
    while (next >= count) {
        next -= count;
    }
    return (uint8_t)next;
}

class AppGachaBannerSettings : public AppBase {
public:
    void onCreate() override
    {
        const SysGacha::Banner &banner = SysGacha::GetBanner();
        phase_ = BannerSettingsPhase::SelectField;
        selected_field_ = 0;
        edit_up_a_ = banner.up6_a_index;
        edit_up_b_ = banner.up6_b_index;
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
        if (delta == 0) {
            return;
        }

        if (phase_ == BannerSettingsPhase::SelectField) {
            selected_field_ = clampInt(selected_field_ - delta, 0, 2);
        } else if (phase_ == BannerSettingsPhase::EditUpA) {
            edit_up_a_ = stepStar6Index(edit_up_a_, -delta);
        } else {
            edit_up_b_ = stepStar6Index(edit_up_b_, -delta);
        }
        dirty_ = true;
    }

    void onKeyShort() override
    {
        if (phase_ == BannerSettingsPhase::SelectField) {
            if (selected_field_ == 0) {
                phase_ = BannerSettingsPhase::EditUpA;
            } else if (selected_field_ == 1) {
                phase_ = BannerSettingsPhase::EditUpB;
            } else {
                appManager.pop();
                return;
            }
            dirty_ = true;
            return;
        }

        SysGacha::SetBannerUp6(edit_up_a_, edit_up_b_);
        const SysGacha::Banner &banner = SysGacha::GetBanner();
        edit_up_a_ = banner.up6_a_index;
        edit_up_b_ = banner.up6_b_index;
        phase_ = BannerSettingsPhase::SelectField;
        dirty_ = true;
    }

    void onKeyLong() override
    {
        if (phase_ == BannerSettingsPhase::SelectField) {
            appManager.pop();
            return;
        }

        const SysGacha::Banner &banner = SysGacha::GetBanner();
        edit_up_a_ = banner.up6_a_index;
        edit_up_b_ = banner.up6_b_index;
        phase_ = BannerSettingsPhase::SelectField;
        dirty_ = true;
    }

private:
    BannerSettingsPhase phase_ = BannerSettingsPhase::SelectField;
    int selected_field_ = 0;
    uint8_t edit_up_a_ = 0;
    uint8_t edit_up_b_ = 1;
    bool dirty_ = true;

    void draw()
    {
        HAL_BeginFrame(UITheme::Color::kBackground);

        if (phase_ == BannerSettingsPhase::SelectField) {
            drawSelectField();
        } else {
            drawEditField();
        }

        HAL_Flush();
    }

    void drawSelectField()
    {
        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(8, 2, UIStrings::GachaBannerSettingsTitle());

        drawMenuLine(0, 18, "UP1", edit_up_a_);
        drawMenuLine(1, 34, "UP2", edit_up_b_);

        HAL_SetTextColor(selected_field_ == 2 ? UITheme::Color::kAccent : UITheme::Gacha::kDim, UITheme::Color::kBackground);
        HAL_DrawText(8, 52, selected_field_ == 2 ? "> 返回" : "  返回");
    }

    void drawMenuLine(int field, int y, const char *label, uint8_t index)
    {
        const SysGacha::Operator *op = SysGacha::GetStar6Operator(index);
        char line[64];
        snprintf(line, sizeof(line), "%s %s %s", selected_field_ == field ? ">" : " ", label, op ? op->name : "未知");

        HAL_SetTextColor(selected_field_ == field ? UITheme::Color::kAccent : UITheme::Gacha::kStar6, UITheme::Color::kBackground);
        HAL_DrawText(8, y, line);
    }

    void drawEditField()
    {
        bool editing_a = (phase_ == BannerSettingsPhase::EditUpA);
        uint8_t index = editing_a ? edit_up_a_ : edit_up_b_;
        const SysGacha::Operator *op = SysGacha::GetStar6Operator(index);
        int count = SysGacha::GetStar6PoolCount();

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(8, 2, editing_a ? "编辑 UP1" : "编辑 UP2");

        char line[64];
        snprintf(line, sizeof(line), "%d/%d %s", (int)index + 1, count, op ? op->name : "未知");
        HAL_SetTextColor(UITheme::Gacha::kStar6, UITheme::Color::kBackground);
        HAL_DrawText(8, 24, line);

        HAL_SetTextColor(UITheme::Gacha::kDim, UITheme::Color::kBackground);
        HAL_DrawText(8, 48, "短按保存  长按取消");
    }
};

} // namespace

AppGachaBannerSettings g_app_gacha_banner_settings;
AppBase *appGachaBannerSettings = &g_app_gacha_banner_settings;
