#include "sys/sys_config.h"

#include <ArduinoJson.h>
#include <string.h>

#include "sys/sys_fs.h"

SysConfig sysConfig;

namespace {

constexpr const char *kConfigPath = "/config.json";

SysGacha::Rarity parseRarity(int value)
{
    switch (value) {
        case 6:
            return SysGacha::Rarity::Star6;
        case 5:
            return SysGacha::Rarity::Star5;
        case 4:
            return SysGacha::Rarity::Star4;
        case 3:
        default:
            return SysGacha::Rarity::Star3;
    }
}

uint8_t clampPullCount(int value)
{
    if (value < 0) return 0;
    if (value > SysGacha::kPullCount) return SysGacha::kPullCount;
    return (uint8_t)value;
}

void copyText(char *dest, size_t dest_size, const char *src)
{
    if (dest == nullptr || dest_size == 0) return;
    if (src == nullptr) src = "";

    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

} // namespace

void SysConfig::load()
{
    String json = SysFS_Read_File(kConfigPath);
    if (json.length() == 0) {
        save();
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.printf("[CONFIG] Parse failed: %s\n", error.c_str());
        return;
    }

    JsonObject gacha = doc["gacha"];
    if (gacha.isNull()) {
        return;
    }

    JsonObject banner = gacha["banner"];
    if (!banner.isNull()) {
        copyText(gacha_banner.title, sizeof(gacha_banner.title), banner["title"] | "标准寻访");
        gacha_banner.up6_a_index = SysGacha::ClampStar6Index(banner["up6_a_index"] | 0);
        gacha_banner.up6_b_index = SysGacha::ClampStar6Index(banner["up6_b_index"] | 1);
        if (gacha_banner.up6_a_index == gacha_banner.up6_b_index) {
            gacha_banner.up6_b_index = SysGacha::NextStar6Index(gacha_banner.up6_a_index);
        }
    }

    JsonObject stats = gacha["stats"];
    if (!stats.isNull()) {
        gacha_stats.total = stats["total"] | 0;
        gacha_stats.star6 = stats["star6"] | 0;
        gacha_stats.star5 = stats["star5"] | 0;
        gacha_stats.star4 = stats["star4"] | 0;
        gacha_stats.star3 = stats["star3"] | 0;
    }

    JsonObject pity = gacha["pity"];
    if (!pity.isNull()) {
        gacha_pity.pulls_since_star6 = pity["pulls_since_star6"] | 0;
        gacha_pity.banner_pulls = pity["banner_pulls"] | 0;
        gacha_pity.first_standard_select_used = pity["first_standard_select_used"] | false;
        gacha_pity.second_standard_select_used = pity["second_standard_select_used"] | false;
    }

    JsonArray last = gacha["last_pulls"];
    gacha_last_pull_count = clampPullCount(last.size());
    for (int i = 0; i < gacha_last_pull_count; ++i) {
        JsonObject item = last[i];
        gacha_last_pulls[i].rarity = parseRarity(item["rarity"] | 3);
        gacha_last_pulls[i].pool_index = item["pool_index"] | 0;
        gacha_last_pulls[i].from_pity = item["from_pity"] | false;
        gacha_last_pulls[i].standard_select_hit = item["standard_select_hit"] | false;
    }

    Serial.println("[CONFIG] Loaded.");
}

void SysConfig::save()
{
    JsonDocument doc;
    JsonObject gacha = doc["gacha"].to<JsonObject>();

    JsonObject banner = gacha["banner"].to<JsonObject>();
    banner["title"] = gacha_banner.title;
    banner["up6_a_index"] = SysGacha::ClampStar6Index(gacha_banner.up6_a_index);
    banner["up6_b_index"] = SysGacha::ClampStar6Index(gacha_banner.up6_b_index);

    JsonObject stats = gacha["stats"].to<JsonObject>();
    stats["total"] = gacha_stats.total;
    stats["star6"] = gacha_stats.star6;
    stats["star5"] = gacha_stats.star5;
    stats["star4"] = gacha_stats.star4;
    stats["star3"] = gacha_stats.star3;

    JsonObject pity = gacha["pity"].to<JsonObject>();
    pity["pulls_since_star6"] = gacha_pity.pulls_since_star6;
    pity["banner_pulls"] = gacha_pity.banner_pulls;
    pity["first_standard_select_used"] = gacha_pity.first_standard_select_used;
    pity["second_standard_select_used"] = gacha_pity.second_standard_select_used;

    JsonArray last = gacha["last_pulls"].to<JsonArray>();
    uint8_t count = clampPullCount(gacha_last_pull_count);
    for (int i = 0; i < count; ++i) {
        JsonObject item = last.add<JsonObject>();
        item["rarity"] = SysGacha::RarityStars(gacha_last_pulls[i].rarity);
        item["pool_index"] = gacha_last_pulls[i].pool_index;
        item["from_pity"] = gacha_last_pulls[i].from_pity;
        item["standard_select_hit"] = gacha_last_pulls[i].standard_select_hit;
    }

    String output;
    serializeJson(doc, output);
    if (SysFS_Write_File(kConfigPath, output.c_str())) {
        Serial.println("[CONFIG] Saved.");
    } else {
        Serial.println("[CONFIG] Save failed.");
    }
}
