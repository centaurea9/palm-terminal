#include "apps/app_menu_base.h"

#include "lang/ui_strings.h"
#include "sys/app_manager.h"
#include "ui/ui_theme.h"

namespace {

MenuItem gacha_root = {
    "GACHA",
    "GACHA",
    nullptr, 0, 0,
    nullptr,
    nullptr, 0,
};

MenuItem gacha_single = {
    "Single",
    "Single",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem gacha_ten = {
    "Ten Pull",
    "Ten",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem gacha_history = {
    "History",
    "History",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem gacha_rules = {
    "Rules",
    "Rules",
    nullptr, 0, 0,
    &gacha_root,
    nullptr, 0,
};

MenuItem *gacha_children[] = {
    &gacha_single,
    &gacha_ten,
    &gacha_history,
    &gacha_rules,
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
