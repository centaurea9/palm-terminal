/**
 * 掌上终端 — 主程序
 *
 * 模块:
 *   - screen/    显示驱动 (ST7789, 76×284 横屏)
 *   - screen/    中英文文字渲染 (U8g2_for_TFT_eSPI)
 *   - menu/      多级菜单
 */

#include "screen/screen.h"
#include "menu.h"

// ===== 动画参数 (方便微调) =====
#define ANIM_STEP    3      // 每帧移动像素数 (24/3=8帧)
#define ANIM_DELAY   16     // 帧间隔 ms (≈60fps)
#define ROW_HEIGHT   24     // 行高 (y2-y1)

// ===== 调试开关 =====
#define AUTO_DEMO    1      // 1=自动循环演示, 0=串口手动控制
#define DEMO_INTERVAL 800   // 自动演示按键间隔 ms

// ===== 菜单状态 =====
static MenuItem *current_menu  = nullptr;   // 当前所在菜单节点
static int selected_index      = 0;         // 选中项在 children[] 中的下标
static bool in_content         = false;     // true=内容页, false=菜单
static MenuItem *content_item  = nullptr;   // 当前内容页对应菜单项

// 动画
static int  anim_offset   = 0;              // 当前偏移 (≠0 表示动画进行中)
static unsigned long anim_tick = 0;         // 上次动画帧的时间

// 返回栈
#define MENU_STACK_MAX 8
static int menu_stack[MENU_STACK_MAX];
static int menu_stack_top = 0;

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println("\n============================================");
    Serial.println("  掌上终端 - 76x284 ST7789");
    Serial.println("============================================");

    screen_init();
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());

    current_menu = menu_root;
    selected_index = 0;    // 默认选中第一项
}

// ===== 绘制右侧 3 行菜单 =====
static void draw_menu() {
    if (!current_menu || current_menu->child_count == 0) return;

    int n  = current_menu->child_count;
    int si = selected_index;
    int dy = anim_offset;                         // 动画 y 偏移

    MenuItem *row1 = (si > 0)      ? current_menu->children[si - 1] : nullptr;
    MenuItem *row2 = current_menu->children[si];
    MenuItem *row3 = (si + 1 < n)  ? current_menu->children[si + 1] : nullptr;

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);

    // ---- 行1 (非选中, x=185) ----
    if (row1) screen_draw_text(185, 7 + dy, row1->title);

    // ---- 行2 (选中, x=175 + 箭头) ----
    if (row2) {
        int arrow_w = u8g2.getUTF8Width(">");
        int gap     = 175 - 160 - arrow_w;
        screen_draw_text(160,              31 + dy, ">");
        screen_draw_text(175,              31 + dy, row2->title);
        int text_w = u8g2.getUTF8Width(row2->title);
        screen_draw_text(175 + text_w + gap, 31 + dy, "<");
    }

    // ---- 行3 (非选中, x=185) ----
    if (row3) screen_draw_text(185, 56 + dy, row3->title);

    // 黄框 (始终在固定位置)
    screen_sprite.drawRect(94, 24, 186, 22, TFT_YELLOW);
}

// ===== 绘制内容页 (叶子节点进入后) =====
static void draw_content() {
    if (!content_item) return;

    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);

    // 右侧显示内容标题 (后续替换为实际天气/数据)
    screen_draw_text(100, 15, content_item->title);
    screen_draw_text(100, 45, "(content area)");
}

// ===== 绘制左侧面板 =====
static void draw_left() {
    // Step 3: 暂时固定显示 (Step 8 联动)
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(22, 15, "星期天");

    u8g2.setFont(u8g2_font_freedoomr25_tn);
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(-3, 40, "10:16");
}

void loop() {
    static bool dirty = true;

    // ---- 动画: 逐帧逼近 0 ----
    if (anim_offset != 0 && millis() - anim_tick >= ANIM_DELAY) {
        anim_tick = millis();
        if (anim_offset > 0) {
            anim_offset -= ANIM_STEP;
            if (anim_offset < 0) anim_offset = 0;
        } else {
            anim_offset += ANIM_STEP;
            if (anim_offset > 0) anim_offset = 0;
        }
        dirty = true;
    }

    // ---- 自动演示 (AUTO_DEMO=1 时生效) ----
#if AUTO_DEMO
    if (anim_offset == 0) {
        static int  demo_dir  = 1;          // 1=向下, -1=向上
        static unsigned long demo_tick = 0;
        if (millis() - demo_tick >= DEMO_INTERVAL) {
            demo_tick = millis();

            if (in_content) {
                // 内容页: 自动退出
                in_content = false;
                content_item = nullptr;
                dirty = true;
                Serial.println("Demo: back to menu");
            } else if (demo_dir > 0) {
                // 向下滚
                if (selected_index + 1 < current_menu->child_count) {
                    selected_index++;
                    anim_offset = +ROW_HEIGHT;
                    anim_tick = millis();
                    dirty = true;
                } else {
                    demo_dir = -1;  // 触底反弹
                }
            } else {
                // 向上滚
                if (selected_index > 0) {
                    selected_index--;
                    anim_offset = -ROW_HEIGHT;
                    anim_tick = millis();
                    dirty = true;
                } else {
                    demo_dir = 1;   // 触顶反弹
                }
            }
        }
        // 清空串口缓冲区 (防止积压)
        while (Serial.available()) Serial.read();
    }
#else
    // ---- 串口输入 (动画期间屏蔽, 防连按撕裂) ----
    if (anim_offset == 0) {
        while (Serial.available()) {
            char c = Serial.read();

            if (in_content) {
                if (c == 'q' || c == 'Q') {
                    in_content = false;
                    content_item = nullptr;
                    dirty = true;
                    Serial.println("Back to menu");
                }
                continue;
            }

            switch (c) {
                case 'w': case 'W':
                    if (selected_index > 0) {
                        selected_index--;
                        anim_offset = -ROW_HEIGHT;  // 从上方滑入
                        anim_tick = millis();
                        dirty = true;
                    }
                    break;

                case 's': case 'S':
                    if (selected_index + 1 < current_menu->child_count) {
                        selected_index++;
                        anim_offset = +ROW_HEIGHT;  // 从下方滑入
                        anim_tick = millis();
                        dirty = true;
                    }
                    break;

                case 'e': case 'E': {
                    MenuItem *sel = current_menu->children[selected_index];
                    if (sel->children && sel->child_count > 0) {
                        menu_stack[menu_stack_top++] = selected_index;
                        current_menu = sel;
                        selected_index = 0;
                        dirty = true;
                        Serial.printf("Enter: %s\n", sel->title);
                    } else {
                        in_content = true;
                        content_item = sel;
                        dirty = true;
                        Serial.printf("Enter: %s (content)\n", sel->title);
                    }
                    break;
                }

                case 'q': case 'Q':
                    if (current_menu->parent) {
                        Serial.printf("Back: %s\n", current_menu->parent->title);
                        current_menu = current_menu->parent;
                        selected_index = (menu_stack_top > 0) ? menu_stack[--menu_stack_top] : 0;
                        dirty = true;
                    }
                    break;
            }
        }
    }
#endif

    // ---- 绘制 (全部画到 Sprite, 最后一把推到屏幕) ----
    if (dirty) {
        dirty = false;
        screen_sprite.fillSprite(TFT_BLACK);  // Sprite 内擦除 (用户看不到)
        draw_left();
        screen_sprite.drawFastVLine(89, 0, 76, TFT_CYAN);
        if (in_content) draw_content();
        else            draw_menu();
        screen_flush();  // 一次性推到屏幕, 零闪烁
    }

    delay(5);
}
