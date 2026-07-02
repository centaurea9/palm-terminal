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

// ===== 菜单状态 =====
static MenuItem *current_menu  = nullptr;   // 当前所在菜单节点
static int selected_index      = 0;         // 选中项在 children[] 中的下标
static bool in_content         = false;     // true=内容页, false=菜单
static MenuItem *content_item  = nullptr;   // 当前内容页对应菜单项

// 返回栈: 记住每层进入前的 selected_index, 返回时恢复
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

    int n = current_menu->child_count;            // 总项数
    int si = selected_index;                      // 选中下标

    MenuItem *row1 = (si > 0)          ? current_menu->children[si - 1] : nullptr;
    MenuItem *row2 = current_menu->children[si];  // 选中项
    MenuItem *row3 = (si + 1 < n)      ? current_menu->children[si + 1] : nullptr;

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);

    // 行1 (非选中, x=185)
    if (row1) screen_draw_text(185, 7, row1->title);

    // 行2 (选中, x=175 + 箭头)
    if (row2) {
        int arrow_w = u8g2.getUTF8Width(">");
        int gap     = 175 - 160 - arrow_w;
        screen_draw_text(160, 31, ">");
        screen_draw_text(175, 31, row2->title);
        int text_w = u8g2.getUTF8Width(row2->title);
        screen_draw_text(175 + text_w + gap, 31, "<");
    }

    // 行3 (非选中, x=185)
    if (row3) screen_draw_text(185, 56, row3->title);

    // 黄框 (焊死在行2)
    tft.drawRect(94, 24, 186, 22, TFT_YELLOW);
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
    static bool dirty = true;  // 脏标记: 状态变了才刷新

    // ---- 串口输入 ----
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

        // 菜单模式
        switch (c) {
            case 'w': case 'W':
                if (selected_index > 0) { selected_index--; dirty = true; }
                break;

            case 's': case 'S':
                if (selected_index + 1 < current_menu->child_count) { selected_index++; dirty = true; }
                break;

            case 'e': case 'E': {
                MenuItem *sel = current_menu->children[selected_index];
                if (sel->children && sel->child_count > 0) {
                    menu_stack[menu_stack_top++] = selected_index;  // 记住当前位置
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
                    // 恢复上级的选中位置
                    selected_index = (menu_stack_top > 0) ? menu_stack[--menu_stack_top] : 0;
                    dirty = true;
                }
                break;
        }
    }

    // ---- 绘制 (仅在脏时) ----
    if (dirty) {
        dirty = false;
        tft.fillScreen(TFT_BLACK);
        draw_left();
        tft.drawFastVLine(89, 0, 76, TFT_CYAN);
        if (in_content) draw_content();
        else            draw_menu();
    }

    delay(5);  // 防 busy-loop
}
