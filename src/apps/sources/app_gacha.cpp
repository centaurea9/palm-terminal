#include "apps/app_menu_base.h"

#include "lang/ui_strings.h"
#include "sys/app_manager.h"
#include "sys/app_registry.h"
#include "ui/ui_theme.h"

namespace {

MenuItem gacha_root = {
    UIStrings::GachaMenuTitle(),
    "寻访",
    nullptr, 0, 0,
    nullptr,
    nullptr, 0,
};

MenuItem gacha_standard = {
    UIStrings::GachaStandardTitle(),
    "标准",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem gacha_history = {
    UIStrings::GachaHistoryTitle(),
    "历史",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem gacha_back = {
    UIStrings::GachaBackText(),
    "返回",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem *gacha_children[] = {
    &gacha_standard,
    &gacha_history,
    &gacha_back,
};

class AppGacha : public AppMenuBase {
public:
    void onCreate() override
    {
        gacha_root.children = gacha_children;
        gacha_root.child_count = sizeof(gacha_children) / sizeof(gacha_children[0]);
        AppMenuBase::onCreate();
    }

protected:
    MenuItem *rootMenu() override
    {
        return &gacha_root;
    }

    void onRootBack() override
    {
        appManager.pop();
    }

    bool onLeafSelected(MenuItem *item) override
    {
        if (item == &gacha_standard) {
            appManager.push(AppId::GachaStandard);
            return true;
        }

        if (item == &gacha_history) {
            appManager.push(AppId::GachaHistory);
            return true;
        }

        if (item == &gacha_back) {
            appManager.pop();
            return true;
        }

        return false;
    }

    void drawLeafContent(MenuItem *item) override
    {
        if (item == nullptr) return;

        const char *title = item->title ? item->title : UIStrings::GachaTitle();
        HAL_SetFont(u8g2_font_wqy16_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(UITheme::Layout::kContentX, UITheme::Layout::kContentTitleY, title);

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_DrawText(UITheme::Layout::kContentX, UITheme::Layout::kContentBodyY, UIStrings::GachaReady());
        HAL_DrawText(UITheme::Layout::kContentX, UITheme::Layout::kContentBodyY + 16, UIStrings::GachaHint());
    }
};

} // namespace

AppGacha g_app_gacha;
AppBase *appGacha = &g_app_gacha;
