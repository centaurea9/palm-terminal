#include "sys/app_registry.h"

extern AppBase *appStandby;
extern AppBase *appMainMenu;
extern AppBase *appGacha;
extern AppBase *appGachaStandard;
extern AppBase *appGachaHistory;

AppBase *AppRegistry_Get(AppId id)
{
    switch (id) {
        case AppId::Standby:
            return appStandby;
        case AppId::MainMenu:
            return appMainMenu;
        case AppId::Gacha:
            return appGacha;
        case AppId::GachaStandard:
            return appGachaStandard;
        case AppId::GachaHistory:
            return appGachaHistory;
        default:
            return nullptr;
    }
}
