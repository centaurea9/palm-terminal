#include "sys/sys_gacha.h"

#include <Arduino.h>

#include "sys/sys_config.h"

namespace SysGacha {
namespace {

namespace Rates {
constexpr int kRateScale = 10000;
constexpr int kStar6Base = 200;
constexpr int kStar5 = 800;
constexpr int kStar4 = 5000;
constexpr int kPityStart = 50;
constexpr int kPityStep = 200;
} // namespace Rates

const Operator kStar6Pool[] = {
    { "鸿雪", Rarity::Star6 },
    { "玛恩纳", Rarity::Star6 },
    { "白铁", Rarity::Star6 },
    { "斥罪", Rarity::Star6 },
    { "焰影苇草", Rarity::Star6 },
    { "伊内丝", Rarity::Star6 },
    { "提丰", Rarity::Star6 },
    { "逻各斯", Rarity::Star6 },
};

const Operator kStar5Pool[] = {
    { "晓歌", Rarity::Star5 },
    { "但书", Rarity::Star5 },
    { "明椒", Rarity::Star5 },
    { "子月", Rarity::Star5 },
    { "和弦", Rarity::Star5 },
    { "火哨", Rarity::Star5 },
    { "青枳", Rarity::Star5 },
    { "海霓", Rarity::Star5 },
};

const Operator kStar4Pool[] = {
    { "夜烟", Rarity::Star4 },
    { "远山", Rarity::Star4 },
    { "杰西卡", Rarity::Star4 },
    { "流星", Rarity::Star4 },
    { "白雪", Rarity::Star4 },
    { "桃金娘", Rarity::Star4 },
    { "红云", Rarity::Star4 },
    { "苏苏洛", Rarity::Star4 },
};

const Operator kStar3Pool[] = {
    { "芬", Rarity::Star3 },
    { "香草", Rarity::Star3 },
    { "翎羽", Rarity::Star3 },
    { "玫兰莎", Rarity::Star3 },
    { "卡缇", Rarity::Star3 },
    { "克洛丝", Rarity::Star3 },
    { "芙蓉", Rarity::Star3 },
    { "安赛尔", Rarity::Star3 },
};

Stats g_stats = {};
PityState g_pity = {};
Banner g_banner = {
    "标准寻访",
    0,
    1,
};
PullResult g_last_pulls[kPullCount] = {};
int g_last_pull_count = 0;

int clampRate(int rate)
{
    if (rate < 0) return 0;
    if (rate > Rates::kRateScale) return Rates::kRateScale;
    return rate;
}

int calcSixRate()
{
    if (g_pity.pulls_since_star6 < Rates::kPityStart) {
        return Rates::kStar6Base;
    }

    int pity_bonus_steps = (int)g_pity.pulls_since_star6 - (Rates::kPityStart - 1);
    return clampRate(Rates::kStar6Base + pity_bonus_steps * Rates::kPityStep);
}

template <size_t N>
const Operator *pickFromPool(const Operator (&pool)[N])
{
    if (N == 0) return nullptr;
    return &pool[random((long)N)];
}

const Operator *pickSixStarOperator(bool *standard_select_hit)
{
    if (standard_select_hit != nullptr) {
        *standard_select_hit = false;
    }

    if (g_pity.banner_pulls >= 300 && !g_pity.second_standard_select_used) {
        g_pity.second_standard_select_used = true;
        if (standard_select_hit != nullptr) {
            *standard_select_hit = true;
        }
        return GetStar6Operator(g_banner.up6_b_index);
    }

    if (g_pity.banner_pulls >= 150 && !g_pity.first_standard_select_used) {
        g_pity.first_standard_select_used = true;
        if (standard_select_hit != nullptr) {
            *standard_select_hit = true;
        }
        return GetStar6Operator(g_banner.up6_a_index);
    }

    return pickFromPool(kStar6Pool);
}

const Operator *pickOperatorByRarity(Rarity rarity)
{
    switch (rarity) {
        case Rarity::Star6:
            return pickSixStarOperator(nullptr);
        case Rarity::Star5:
            return pickFromPool(kStar5Pool);
        case Rarity::Star4:
            return pickFromPool(kStar4Pool);
        case Rarity::Star3:
        default:
            return pickFromPool(kStar3Pool);
    }
}

template <size_t N>
uint8_t poolIndexForOperator(const Operator (&pool)[N], const Operator *op)
{
    if (op == nullptr) return 0;

    for (size_t i = 0; i < N; ++i) {
        if (op == &pool[i]) {
            return (uint8_t)i;
        }
    }

    return 0;
}

uint8_t poolIndexForResult(const PullResult &result)
{
    switch (result.rarity) {
        case Rarity::Star6:
            return poolIndexForOperator(kStar6Pool, result.op);
        case Rarity::Star5:
            return poolIndexForOperator(kStar5Pool, result.op);
        case Rarity::Star4:
            return poolIndexForOperator(kStar4Pool, result.op);
        case Rarity::Star3:
        default:
            return poolIndexForOperator(kStar3Pool, result.op);
    }
}

template <size_t N>
const Operator *operatorFromPoolIndex(const Operator (&pool)[N], uint8_t pool_index)
{
    if (N == 0) return nullptr;
    if (pool_index >= N) pool_index = 0;
    return &pool[pool_index];
}

const Operator *operatorFromSavedIndex(Rarity rarity, uint8_t pool_index)
{
    switch (rarity) {
        case Rarity::Star6:
            return operatorFromPoolIndex(kStar6Pool, pool_index);
        case Rarity::Star5:
            return operatorFromPoolIndex(kStar5Pool, pool_index);
        case Rarity::Star4:
            return operatorFromPoolIndex(kStar4Pool, pool_index);
        case Rarity::Star3:
        default:
            return operatorFromPoolIndex(kStar3Pool, pool_index);
    }
}

void updateStats(Rarity rarity)
{
    ++g_stats.total;

    switch (rarity) {
        case Rarity::Star6:
            ++g_stats.star6;
            break;
        case Rarity::Star5:
            ++g_stats.star5;
            break;
        case Rarity::Star4:
            ++g_stats.star4;
            break;
        case Rarity::Star3:
            ++g_stats.star3;
            break;
    }
}

void updatePity(Rarity rarity)
{
    if (rarity == Rarity::Star6) {
        g_pity.pulls_since_star6 = 0;
        return;
    }

    if (g_pity.pulls_since_star6 < UINT16_MAX) {
        ++g_pity.pulls_since_star6;
    }
}

PullResult rollSingle()
{
    int six_rate = calcSixRate();
    if (g_pity.banner_pulls < UINT16_MAX) {
        ++g_pity.banner_pulls;
    }

    int r = random(Rates::kRateScale);

    Rarity rarity = Rarity::Star3;
    if (r < six_rate) {
        rarity = Rarity::Star6;
    } else if (r < six_rate + Rates::kStar5) {
        rarity = Rarity::Star5;
    } else if (r < six_rate + Rates::kStar5 + Rates::kStar4) {
        rarity = Rarity::Star4;
    }

    bool standard_select_hit = false;
    const Operator *op = (rarity == Rarity::Star6)
        ? pickSixStarOperator(&standard_select_hit)
        : pickOperatorByRarity(rarity);

    PullResult result = {
        op,
        rarity,
        rarity == Rarity::Star6 && six_rate > Rates::kStar6Base,
        standard_select_hit,
    };

    updateStats(rarity);
    updatePity(rarity);
    return result;
}

} // namespace

void ExecuteTenPull(PullResult *out_results, int out_count)
{
    g_last_pull_count = kPullCount;

    for (int i = 0; i < kPullCount; ++i) {
        g_last_pulls[i] = rollSingle();
        if (out_results != nullptr && i < out_count) {
            out_results[i] = g_last_pulls[i];
        }
    }

    SaveStateToConfig();
}

void LoadStateFromConfig()
{
    g_banner.title = sysConfig.gacha_banner.title;
    g_banner.up6_a_index = ClampStar6Index(sysConfig.gacha_banner.up6_a_index);
    g_banner.up6_b_index = ClampStar6Index(sysConfig.gacha_banner.up6_b_index);
    if (g_banner.up6_a_index == g_banner.up6_b_index) {
        g_banner.up6_b_index = NextStar6Index(g_banner.up6_a_index);
    }
    sysConfig.gacha_banner.up6_a_index = g_banner.up6_a_index;
    sysConfig.gacha_banner.up6_b_index = g_banner.up6_b_index;

    g_stats = sysConfig.gacha_stats;
    g_pity = sysConfig.gacha_pity;
    g_last_pull_count = sysConfig.gacha_last_pull_count;
    if (g_last_pull_count < 0) {
        g_last_pull_count = 0;
    }
    if (g_last_pull_count > kPullCount) {
        g_last_pull_count = kPullCount;
    }

    for (int i = 0; i < g_last_pull_count; ++i) {
        const SysConfig::GachaLastPullSave &saved = sysConfig.gacha_last_pulls[i];
        g_last_pulls[i] = {
            operatorFromSavedIndex(saved.rarity, saved.pool_index),
            saved.rarity,
            saved.from_pity,
            saved.standard_select_hit,
        };
    }
}

void SaveStateToConfig()
{
    sysConfig.gacha_banner.up6_a_index = g_banner.up6_a_index;
    sysConfig.gacha_banner.up6_b_index = g_banner.up6_b_index;
    sysConfig.gacha_stats = g_stats;
    sysConfig.gacha_pity = g_pity;
    sysConfig.gacha_last_pull_count = (uint8_t)g_last_pull_count;

    for (int i = 0; i < g_last_pull_count && i < kPullCount; ++i) {
        sysConfig.gacha_last_pulls[i].rarity = g_last_pulls[i].rarity;
        sysConfig.gacha_last_pulls[i].pool_index = poolIndexForResult(g_last_pulls[i]);
        sysConfig.gacha_last_pulls[i].from_pity = g_last_pulls[i].from_pity;
        sysConfig.gacha_last_pulls[i].standard_select_hit = g_last_pulls[i].standard_select_hit;
    }

    sysConfig.save();
}

const Banner &GetBanner()
{
    return g_banner;
}

void SetBannerUp6(uint8_t up_a_index, uint8_t up_b_index)
{
    g_banner.up6_a_index = ClampStar6Index(up_a_index);
    g_banner.up6_b_index = ClampStar6Index(up_b_index);
    if (g_banner.up6_a_index == g_banner.up6_b_index) {
        g_banner.up6_b_index = NextStar6Index(g_banner.up6_a_index);
    }

    sysConfig.gacha_banner.up6_a_index = g_banner.up6_a_index;
    sysConfig.gacha_banner.up6_b_index = g_banner.up6_b_index;
    sysConfig.save();
}

int GetStar6PoolCount()
{
    return sizeof(kStar6Pool) / sizeof(kStar6Pool[0]);
}

const Operator *GetStar6Operator(int index)
{
    int count = GetStar6PoolCount();
    if (count <= 0) {
        return nullptr;
    }

    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    return &kStar6Pool[index];
}

uint8_t ClampStar6Index(uint8_t index)
{
    int count = GetStar6PoolCount();
    if (count <= 0) {
        return 0;
    }

    if (index >= count) {
        return (uint8_t)(count - 1);
    }
    return index;
}

uint8_t NextStar6Index(uint8_t index)
{
    int count = GetStar6PoolCount();
    if (count <= 1) {
        return 0;
    }

    return (uint8_t)((ClampStar6Index(index) + 1) % count);
}

const Stats &GetStats()
{
    return g_stats;
}

const PityState &GetPityState()
{
    return g_pity;
}

const PullResult *GetLastPulls()
{
    return g_last_pulls;
}

int GetLastPullCount()
{
    return g_last_pull_count;
}

int RarityStars(Rarity rarity)
{
    return (int)rarity;
}

} // namespace SysGacha
