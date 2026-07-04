#ifndef SYS_APP_REGISTRY_H
#define SYS_APP_REGISTRY_H

#include <stdint.h>
#include "sys/app_base.h"

enum class AppId : uint8_t {
    Standby,
    MainMenu,
    Gacha,
    GachaStandard,
    GachaHistory,
    GachaBannerSettings,
};

AppBase *AppRegistry_Get(AppId id);

#endif
