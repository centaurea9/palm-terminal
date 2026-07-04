#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <stdint.h>

#include "sys/sys_gacha.h"

class SysConfig {
public:
    struct GachaBannerSave {
        char title[24];
        uint8_t up6_a_index;
        uint8_t up6_b_index;
    };

    struct GachaLastPullSave {
        SysGacha::Rarity rarity;
        uint8_t pool_index;
        bool from_pity;
        bool standard_select_hit;
    };

    GachaBannerSave gacha_banner = {
        "标准寻访",
        0,
        1,
    };
    SysGacha::Stats gacha_stats = {};
    SysGacha::PityState gacha_pity = {};
    GachaLastPullSave gacha_last_pulls[SysGacha::kPullCount] = {};
    uint8_t gacha_last_pull_count = 0;

    void load();
    void save();
};

extern SysConfig sysConfig;

#endif
