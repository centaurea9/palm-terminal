#include <Arduino.h>
#include "screen/screen.h"
#include "menu.h"
#include "input/ec11.h"
#include <math.h>

#define ANIM_SPEED                0.22f
#define VISUAL_EPS                0.02f
#define FRAME_FAST_MS             16
#define VISIBLE_RADIUS            3
#define MENU_DIVIDER_X            89
#define MENU_CURVE_FACTOR         7.0f
#define MENU_CURVE_SPEED_BOOST    6.5f
#define MENU_SLING_SPEED_BOOST    4.0f
#define MENU_CENTER_FLING_X       12
#define MENU_CENTER_MIN_LEFT_GAP  28
#define MENU_BOX_PAD_X            4
#define MENU_SCROLL_BAR_W         4
#define MENU_BOX_H                22
#define MENU_ITEM_SPACING_Y       18

#define AUTO_DEMO                 0
#define DEMO_INTERVAL             1200
#define DEMO_BURST_INTERVAL       4800
#define DEMO_BURST_STEP_MS        70
#define DEMO_BURST_STEPS          5

#define EC11_PIN_A                25
#define EC11_PIN_B                26
#define EC11_PIN_SW               27
#define EC11_LONG_PRESS_MS        700

static MenuItem *current_menu = nullptr;
static int selected_index = 0;
static int menu_target_position = 0;
static float visual_index = 0.0f;
static float menu_curve_energy = 0.0f;
static float menu_sling_display = 0.0f;
static bool in_content = false;
static MenuItem *content_item = nullptr;
static EC11Input encoder;

static unsigned long anim_ts = 0;
static unsigned long menu_last_input_ts = 0;
static unsigned long menu_curve_ts = 0;
static unsigned long menu_sling_ts = 0;

#define MENU_STACK_MAX 8
static int menu_stack[MENU_STACK_MAX];
static int menu_stack_top = 0;

static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static int clampi(int value, int min_value, int max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static uint16_t fade_color(uint16_t base, float intensity) {
    intensity = clampf(intensity, 0.0f, 1.0f);

    uint8_t r8 = (uint8_t)((((base >> 11) & 0x1F) * 255 / 31) * intensity);
    uint8_t g8 = (uint8_t)((((base >> 5) & 0x3F) * 255 / 63) * intensity);
    uint8_t b8 = (uint8_t)(((base & 0x1F) * 255 / 31) * intensity);
    return tft.color565(r8, g8, b8);
}

static float smoothstep(float value) {
    value = clampf(value, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

static const char *menu_title_text() {
    if (!current_menu || !current_menu->title) return "";
    return current_menu->title;
}

static const char *menu_left_focus_text() {
    if (in_content && content_item && content_item->left_text) {
        return content_item->left_text;
    }
    if (current_menu && current_menu->child_count > 0) {
        MenuItem *item = current_menu->children[selected_index];
        if (item && item->left_text) return item->left_text;
    }
    return "";
}

static int menu_count() {
    return current_menu ? current_menu->child_count : 0;
}

static void reset_menu_view(int index) {
    selected_index = index;
    menu_target_position = index;
    visual_index = (float)index;
    menu_curve_energy = 0.0f;
    menu_sling_display = 0.0f;

    unsigned long now = millis();
    anim_ts = now;
    menu_last_input_ts = now;
    menu_curve_ts = now;
    menu_sling_ts = now;
}

static bool menu_is_settled() {
    return fabsf(visual_index - (float)menu_target_position) < 0.05f &&
           menu_sling_display < 0.03f;
}

static void move_selection(int delta) {
    int count = menu_count();
    if (count <= 0) return;

    int next = clampi(menu_target_position + delta, 0, count - 1);
    if (next == menu_target_position) return;

    unsigned long now = millis();
    unsigned long gap = (menu_last_input_ts == 0) ? 80 : (now - menu_last_input_ts);
    menu_last_input_ts = now;
    if (gap < 8) gap = 8;

    float speed_factor = 80.0f / (float)gap;
    float impulse = fminf(0.55f, 0.12f * fabsf((float)delta) + 0.10f * speed_factor);
    menu_curve_energy = clampf(menu_curve_energy + impulse, 0.0f, 1.0f);

    selected_index = next;
    menu_target_position = next;
}

static void close_content(bool &dirty) {
    if (!in_content) return;

    in_content = false;
    content_item = nullptr;
    dirty = true;
    Serial.println("Close content");
}

static bool update_curve_energy() {
    unsigned long now = millis();
    if (menu_curve_ts == 0) {
        menu_curve_ts = now;
        return false;
    }

    unsigned long elapsed = now - menu_curve_ts;
    menu_curve_ts = now;
    if (elapsed == 0 || menu_curve_energy <= 0.0f) return false;

    float old = menu_curve_energy;
    menu_curve_energy -= (float)elapsed * 0.0028f;
    if (menu_curve_energy < 0.0f) menu_curve_energy = 0.0f;
    return fabsf(old - menu_curve_energy) > 0.01f;
}

static float target_menu_sling_strength() {
    float pending = fabsf((float)menu_target_position - visual_index);
    if (pending <= 0.015f || menu_curve_energy <= 0.01f) return 0.0f;

    float motion_gate = smoothstep(pending / 0.75f);
    return menu_curve_energy * motion_gate;
}

static bool update_menu_sling_display() {
    unsigned long now = millis();
    if (menu_sling_ts == 0) {
        menu_sling_ts = now;
        return false;
    }

    unsigned long elapsed = now - menu_sling_ts;
    menu_sling_ts = now;
    if (elapsed == 0) return false;
    if (elapsed > 48) elapsed = 48;

    float target = target_menu_sling_strength();
    float old = menu_sling_display;
    float rate = (target > menu_sling_display) ? 0.018f : 0.010f;
    float alpha = 1.0f - expf(-(float)elapsed * rate);
    menu_sling_display += (target - menu_sling_display) * alpha;

    if (menu_sling_display < 0.004f && target <= 0.001f) menu_sling_display = 0.0f;
    menu_sling_display = clampf(menu_sling_display, 0.0f, 1.0f);
    return fabsf(old - menu_sling_display) > 0.002f;
}

static float menu_vertical_offset(float offset, int spacing_y, float sling_strength) {
    float distance = fabsf(offset);
    if (distance <= 0.001f) return 0.0f;

    float sign = (offset >= 0.0f) ? 1.0f : -1.0f;
    float first_gap = 1.14f + sling_strength * 0.08f;
    float outer_gap = 0.74f + sling_strength * 0.06f;
    float units = 0.0f;

    if (distance <= 1.0f) {
        units = distance * first_gap;
    } else {
        units = first_gap + (distance - 1.0f) * outer_gap;
    }

    return sign * (float)spacing_y * units;
}

static float menu_curve_offset_x(float offset) {
    float distance = fabsf(offset);
    if (distance <= 0.001f) return 0.0f;

    float d = fminf(distance, 2.7f);
    return powf(d, 1.62f) * MENU_CURVE_FACTOR;
}

static float menu_sling_offset_x(float offset) {
    float distance = fabsf(offset);
    if (distance <= 0.05f || menu_sling_display <= 0.01f) return 0.0f;

    float d = fminf(distance, 3.0f);
    float speed_curve = powf(d, 1.55f) * MENU_CURVE_SPEED_BOOST;
    float speed_sling = powf(d, 1.12f) * MENU_SLING_SPEED_BOOST;
    return -(speed_curve + speed_sling) * menu_sling_display;
}

static void draw_menu_wheel(int count, int right_panel_x, int right_panel_w, int center_y) {
    const int sw = tft.width();
    const int sh = tft.height();
    const int box_x = right_panel_x + MENU_BOX_PAD_X;
    const int box_w = right_panel_w - MENU_BOX_PAD_X * 2;
    const int box_h = MENU_BOX_H;
    const int box_y = center_y - box_h / 2;
    const int tri_x = box_x + box_w - 6;
    const int tri_size = 7;
    const int item_spacing_y = MENU_ITEM_SPACING_Y;

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    int line_h = u8g2.getFontAscent() - u8g2.getFontDescent();
    const char *selected_text = current_menu->children[selected_index]->title;
    int selected_width = u8g2.getUTF8Width(selected_text);
    if (selected_width < 12) selected_width = 12;

    int rest_center_x = right_panel_x + right_panel_w / 2;
    int speed_center_x = rest_center_x - (int)(menu_sling_display * MENU_CENTER_FLING_X);
    int min_center_x = right_panel_x + MENU_CENTER_MIN_LEFT_GAP + selected_width / 2;
    int max_center_x = sw - selected_width / 2 - 12;
    int center_x = clampi(speed_center_x, min_center_x, max_center_x);

    screen_sprite.drawRect(box_x, box_y, box_w, box_h, TFT_YELLOW);
    screen_sprite.fillTriangle(tri_x, center_y - tri_size, tri_x, center_y + tri_size, tri_x - tri_size - 1, center_y, TFT_CYAN);

    int block_x = center_x - selected_width / 2 - 6 - MENU_SCROLL_BAR_W;
    int block_min_x = box_x + 4;
    if (block_x < block_min_x) block_x = block_min_x;
    screen_sprite.fillRect(block_x, box_y + 2, MENU_SCROLL_BAR_W, box_h - 4, TFT_CYAN);

    int base_idx = (int)roundf(visual_index);
    for (int i = base_idx - VISIBLE_RADIUS; i <= base_idx + VISIBLE_RADIUS; ++i) {
        if (i < 0 || i >= count) continue;

        float offset = (float)i - visual_index;
        float distance = fabsf(offset);
        int item_y = center_y + (int)menu_vertical_offset(offset, item_spacing_y, menu_sling_display);
        if (item_y < -item_spacing_y || item_y > sh + item_spacing_y) continue;

        const char *text = current_menu->children[i]->title;
        int text_width = u8g2.getUTF8Width(text);
        int item_x = center_x - text_width / 2 + (int)(menu_curve_offset_x(offset) + menu_sling_offset_x(offset));
        item_x = clampi(item_x, right_panel_x + 2, sw - text_width - 5);

        float intensity = 1.0f - distance * 0.40f;
        if (intensity < 0.18f) intensity = 0.18f;
        uint16_t color = (distance < 0.55f) ? TFT_CYAN : fade_color(TFT_CYAN, intensity);

        screen_text_color(color, TFT_BLACK);
        screen_draw_text(item_x, item_y - line_h / 2, text);
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== Palm Terminal ===");
    screen_init();
    EC11Config encoder_config;
    encoder_config.pin_a = EC11_PIN_A;
    encoder_config.pin_b = EC11_PIN_B;
    encoder_config.pin_button = EC11_PIN_SW;
    encoder_config.long_press_ms = EC11_LONG_PRESS_MS;
    encoder.begin(encoder_config);
    Serial.printf("Display: %dx%d\n", tft.width(), tft.height());
    Serial.printf("EC11 pins: A=%d, B=%d, SW=%d\n", EC11_PIN_A, EC11_PIN_B, EC11_PIN_SW);
    Serial.println("EC11: CW=up, CCW=down, click=enter, hold=back");
    current_menu = menu_root;
    reset_menu_view(0);
}

static void draw_left() {
    screen_text_color(TFT_CYAN, TFT_BLACK);

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    screen_draw_text(8, 4, menu_title_text());

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    screen_draw_text(8, 18, menu_left_focus_text());

    u8g2.setFont(u8g2_font_freedoomr25_tn);
    screen_draw_text(-3, 40, "10:16");
}

static void draw_menu() {
    int count = menu_count();
    if (count <= 0) return;

    const int right_panel_x = MENU_DIVIDER_X + 1;
    const int right_panel_w = tft.width() - right_panel_x;
    const int center_y = tft.height() / 2;
    draw_menu_wheel(count, right_panel_x, right_panel_w, center_y);
}

static void draw_content() {
    if (!content_item) return;

    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    screen_text_color(TFT_CYAN, TFT_BLACK);
    screen_draw_text(100, 15, content_item->title);
    screen_draw_text(100, 45, "(content area)");
}

static void update_animation(bool &dirty) {
    if (in_content) return;

    float target = (float)menu_target_position;
    float diff = target - visual_index;
    if (fabsf(diff) <= VISUAL_EPS) {
        if (visual_index != target) {
            visual_index = target;
            dirty = true;
        }
        return;
    }

    unsigned long now = millis();
    unsigned long dt = now - anim_ts;
    if (dt < FRAME_FAST_MS) return;

    anim_ts = now;
    if (dt > 48) dt = 48;

    float base_alpha = 0.20f + fminf(menu_curve_energy, 1.0f) * 0.07f;
    float alpha = 1.0f - powf(1.0f - base_alpha, (float)dt / 16.0f);
    visual_index += diff * alpha;
    if (fabsf(target - visual_index) <= VISUAL_EPS) {
        visual_index = target;
    }
    dirty = true;
}

static void handle_menu_enter(bool &dirty) {
    MenuItem *sel = current_menu->children[selected_index];
    if (sel->children && sel->child_count > 0) {
        if (menu_stack_top < MENU_STACK_MAX) {
            menu_stack[menu_stack_top++] = selected_index;
        }
        current_menu = sel;
        reset_menu_view(0);
        dirty = true;
        Serial.printf("Enter: %s\n", sel->title);
        return;
    }

    in_content = true;
    content_item = sel;
    dirty = true;
}

static void handle_menu_back(bool &dirty) {
    if (!current_menu || !current_menu->parent) return;

    current_menu = current_menu->parent;
    int restore_index = 0;
    if (menu_stack_top > 0) {
        restore_index = menu_stack[--menu_stack_top];
    }
    reset_menu_view(restore_index);
    dirty = true;
    Serial.println("Back");
}

static void handle_encoder_input(bool &dirty) {
    encoder.update();

    if (!in_content) {
        int steps = encoder.consume_steps();
        if (steps != 0) {
            // Recommended wiring: clockwise produces positive steps.
            move_selection(-steps);
            dirty = true;
        }
    } else {
        encoder.consume_steps();
    }

    if (encoder.consume_click()) {
        if (in_content) {
            close_content(dirty);
        } else {
            handle_menu_enter(dirty);
        }
    }

    if (encoder.consume_long_press()) {
        if (in_content) {
            close_content(dirty);
        } else {
            handle_menu_back(dirty);
        }
    }
}

static void handle_serial_input(bool &dirty) {
    while (Serial.available()) {
        char c = Serial.read();

        if (in_content) {
            if (c == 'q' || c == 'Q') {
                close_content(dirty);
            }
            continue;
        }

        switch (c) {
            case 'w':
            case 'W':
                move_selection(-1);
                dirty = true;
                break;

            case 's':
            case 'S':
                move_selection(+1);
                dirty = true;
                break;

            case 'e':
            case 'E':
                handle_menu_enter(dirty);
                break;

            case 'q':
            case 'Q':
                handle_menu_back(dirty);
                break;
        }
    }
}

void loop() {
    static bool dirty = true;

    handle_encoder_input(dirty);

#if AUTO_DEMO
    {
        static int demo_dir = 1;
        static unsigned long demo_tick = 0;
        static unsigned long burst_tick = 0;
        static unsigned long burst_step_tick = 0;
        static int burst_steps_left = 0;

        unsigned long now = millis();

        if (burst_steps_left > 0) {
            if (now - burst_step_tick >= DEMO_BURST_STEP_MS) {
                burst_step_tick = now;
                if (demo_dir > 0) {
                    if (selected_index + 1 < menu_count()) {
                        move_selection(+1);
                        burst_steps_left--;
                    } else {
                        demo_dir = -1;
                        burst_steps_left = 0;
                    }
                } else {
                    if (selected_index > 0) {
                        move_selection(-1);
                        burst_steps_left--;
                    } else {
                        demo_dir = 1;
                        burst_steps_left = 0;
                    }
                }

                if (burst_steps_left <= 0) {
                    demo_tick = now;
                    burst_tick = now;
                }
            }
        } else if (now - burst_tick >= DEMO_BURST_INTERVAL && menu_is_settled() && !in_content) {
            burst_tick = now;
            burst_step_tick = now;
            burst_steps_left = DEMO_BURST_STEPS;
        } else if (now - demo_tick >= DEMO_INTERVAL && menu_is_settled()) {
            demo_tick = now;
            if (in_content) {
                in_content = false;
                content_item = nullptr;
                dirty = true;
            } else if (demo_dir > 0) {
                if (selected_index + 1 < menu_count()) {
                    move_selection(+1);
                } else {
                    demo_dir = -1;
                }
            } else {
                if (selected_index > 0) {
                    move_selection(-1);
                } else {
                    demo_dir = 1;
                }
            }
        }

        while (Serial.available()) {
            Serial.read();
        }
    }
#endif

    handle_serial_input(dirty);

    if (update_curve_energy()) dirty = true;
    if (update_menu_sling_display()) dirty = true;
    update_animation(dirty);

    if (dirty) {
        dirty = false;
        screen_sprite.fillSprite(TFT_BLACK);
        draw_left();
        screen_sprite.drawFastVLine(MENU_DIVIDER_X, 0, tft.height(), TFT_CYAN);
        if (in_content) {
            draw_content();
        } else {
            draw_menu();
        }
        screen_flush();
    }

    delay(1);
}
