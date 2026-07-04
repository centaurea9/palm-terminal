#include "apps/app_menu_base.h"

#include "menu.h"
#include "sys/app_manager.h"

extern MenuItem item_func4;

class AppMainMenu : public AppMenuBase {
protected:
    MenuItem *rootMenu() override
    {
        return menu_root;
    }

    bool onLeafSelected(MenuItem *item) override
    {
        if (item == &item_func4) {
            appManager.push(AppId::Gacha);
            return true;
        }
        return false;
    }

    void onRootBack() override
    {
        appManager.launch(AppId::Standby);
    }
};

AppMainMenu g_app_main_menu;
AppBase *appMainMenu = &g_app_main_menu;
