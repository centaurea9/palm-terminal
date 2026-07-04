#include "sys/app_registry.h"

extern AppBase *appStandby;
extern AppBase *appMainMenu;
extern AppBase *appGacha;

AppBase *AppRegistry_Get(AppId id)
{
    switch (id) {
        case AppId::Standby:
            return appStandby;
        case AppId::MainMenu:
            return appMainMenu;
        case AppId::Gacha:
            return appGacha;
        default:
            return nullptr;
    }
}
