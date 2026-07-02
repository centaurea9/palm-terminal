/**
 * 多级菜单 — 数据结构
 *
 * 屏幕布局 (284×76 横屏):
 *   左面板 0~89px  — 随选中项联动
 *   右面板 90~284px — 3 行菜单，选中行 x=175，非选中 x=185
 */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>

struct MenuItem {
    const char *title;        // 菜单文字 (UTF-8, 右侧 3 行)
    const char *left_text;    // 选中时左侧显示的文字

    // 图标 (预留, 1-bit bitmap, nullptr 不画)
    const uint8_t *left_icon;
    int icon_w;
    int icon_h;

    // 层级关系
    struct MenuItem *parent;         // 父菜单 (顶层 = nullptr)
    struct MenuItem **children;      // 子菜单数组 (叶子 = nullptr)
    int child_count;                 // 子菜单数量 (叶子 = 0)
};

// 菜单树根节点 (在 menu.cpp 中定义)
extern struct MenuItem *menu_root;

#endif
