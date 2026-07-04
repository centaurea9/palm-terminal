#ifndef SYS_GACHA_H
#define SYS_GACHA_H

#include <stdint.h>

namespace SysGacha {

enum class Rarity : uint8_t {
    Star3 = 3,
    Star4 = 4,
    Star5 = 5,
    Star6 = 6,
};

struct Operator {
    const char *name;
    Rarity rarity;
};

struct PullResult {
    const Operator *op;
    Rarity rarity;
    bool from_pity;
    bool standard_select_hit;
};

struct Stats {
    uint32_t total;
    uint32_t star6;
    uint32_t star5;
    uint32_t star4;
    uint32_t star3;
};

struct PityState {
    uint16_t pulls_since_star6;
    uint16_t banner_pulls;
    bool first_standard_select_used;
    bool second_standard_select_used;
};

struct Banner {
    const char *title;
    uint8_t up6_a_index;
    uint8_t up6_b_index;
};

constexpr int kPullCount = 10;

void ExecuteTenPull(PullResult *out_results, int out_count);
void LoadStateFromConfig();
void SaveStateToConfig();

const Banner &GetBanner();
void SetBannerUp6(uint8_t up_a_index, uint8_t up_b_index);

int GetStar6PoolCount();
const Operator *GetStar6Operator(int index);
uint8_t ClampStar6Index(uint8_t index);
uint8_t NextStar6Index(uint8_t index);

const Stats &GetStats();
const PityState &GetPityState();
const PullResult *GetLastPulls();
int GetLastPullCount();

int RarityStars(Rarity rarity);

} // namespace SysGacha

#endif
