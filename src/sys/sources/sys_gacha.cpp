#include "sys/sys_gacha.h"

#include <Arduino.h>

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

const Operator *pickOperatorByRarity(Rarity rarity)
{
    switch (rarity) {
        case Rarity::Star6:
            return pickFromPool(kStar6Pool);
        case Rarity::Star5:
            return pickFromPool(kStar5Pool);
        case Rarity::Star4:
            return pickFromPool(kStar4Pool);
        case Rarity::Star3:
        default:
            return pickFromPool(kStar3Pool);
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
    if (g_pity.banner_pulls < UINT16_MAX) {
        ++g_pity.banner_pulls;
    }

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
    int r = random(Rates::kRateScale);

    Rarity rarity = Rarity::Star3;
    if (r < six_rate) {
        rarity = Rarity::Star6;
    } else if (r < six_rate + Rates::kStar5) {
        rarity = Rarity::Star5;
    } else if (r < six_rate + Rates::kStar5 + Rates::kStar4) {
        rarity = Rarity::Star4;
    }

    PullResult result = {
        pickOperatorByRarity(rarity),
        rarity,
        rarity == Rarity::Star6 && six_rate > Rates::kStar6Base,
        false,
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
