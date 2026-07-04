#include "sys/app_base.h"

#include "hal/hal.h"
#include "lang/ui_strings.h"
#include "sys/app_manager.h"
#include "ui/ui_theme.h"

class AppStandby : public AppBase {
public:
    void onCreate() override
    {
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
        HAL_BeginFrame(UITheme::Color::kBackground);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);

        HAL_SetFont(u8g2_font_wqy16_t_gb2312);
        HAL_DrawText(16, 10, UIStrings::StandbyTitle());

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_DrawText(16, 36, UIStrings::StandbyHint());
        HAL_DrawText(16, 54, UIStrings::StandbySubHint());
        HAL_Flush();
    }

    void onDestroy() override {}

    void onKnob(int delta) override
    {
        (void)delta;
    }

    void onKeyShort() override
    {
        appManager.launch(AppId::MainMenu);
    }

    void onKeyLong() override
    {
        appManager.launch(AppId::MainMenu);
    }

private:
    bool dirty_ = true;
};

AppStandby g_app_standby;
AppBase *appStandby = &g_app_standby;
