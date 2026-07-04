/**
 * 多级菜单 — 菜单树数据
 *
 * 结构:
 *   主菜单
 *   ├── 天气预报 (内容页 — 今日+未来两天)
 *   ├── 干员寻访 → 公开招募 / 限定寻访
 *   └── 系统设置 → 背光亮度 / WiFi设置 / 关于本机
 */

#include "menu.h"

// ============================================================
// 前向声明
// ============================================================
extern MenuItem item_weather;
extern MenuItem item_recruit;
extern MenuItem item_settings;
extern MenuItem item_public_recruit;
extern MenuItem item_limited_recruit;
extern MenuItem item_backlight;
extern MenuItem item_wifi;
extern MenuItem item_about;
extern MenuItem item_func4;
extern MenuItem item_func5;
extern MenuItem item_func6;
extern MenuItem root_node;

// ============================================================
// children 数组
// ============================================================
static MenuItem *children_recruit[]  = { &item_public_recruit, &item_limited_recruit };
static MenuItem *children_settings[] = { &item_backlight, &item_wifi, &item_about };
static MenuItem *children_root[]     = { &item_weather, &item_recruit, &item_settings,
                                         &item_func4, &item_func5, &item_func6 };

// ============================================================
// 菜单项定义
// ============================================================

// ---- 叶子节点 ----

MenuItem item_weather = {
    "功能1",
    "功能1",
    nullptr, 0, 0,
    &root_node,
    nullptr, 0,             // 叶子 → 按E进入内容页
};

MenuItem item_public_recruit = {
    "公开招募",
    "公招",
    nullptr, 0, 0,
    &item_recruit,
    nullptr, 0,
};

MenuItem item_limited_recruit = {
    "限定寻访",
    "限定",
    nullptr, 0, 0,
    &item_recruit,
    nullptr, 0,
};

MenuItem item_backlight = {
    "背光亮度",
    "背光",
    nullptr, 0, 0,
    &item_settings,
    nullptr, 0,
};

MenuItem item_wifi = {
    "WiFi设置",
    "WiFi",
    nullptr, 0, 0,
    &item_settings,
    nullptr, 0,
};

MenuItem item_about = {
    "关于本机",
    "关于",
    nullptr, 0, 0,
    &item_settings,
    nullptr, 0,
};

// ---- 父菜单节点 ----

MenuItem item_recruit = {
    "功能2",
    "功能2",
    nullptr, 0, 0,
    &root_node,
    children_recruit, 2,
};

MenuItem item_settings = {
    "功能3",
    "功能3",
    nullptr, 0, 0,
    &root_node,
    children_settings, 3,
};

// ---- 新增叶子 ----

MenuItem item_func4 = {
    "GACHA",
    "GACHA",
    nullptr, 0, 0,
    &root_node,
    nullptr, 0,
};

MenuItem item_func5 = {
    "功能5",
    "功能5",
    nullptr, 0, 0,
    &root_node,
    nullptr, 0,
};

MenuItem item_func6 = {
    "功能6",
    "功能6",
    nullptr, 0, 0,
    &root_node,
    nullptr, 0,
};

// ---- 根节点 ----

MenuItem root_node = {
    "主菜单",
    "星期天",
    nullptr, 0, 0,
    nullptr,
    children_root, 6,
};

MenuItem *menu_root = &root_node;
