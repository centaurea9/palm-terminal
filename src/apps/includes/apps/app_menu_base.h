#ifndef APPS_APP_MENU_BASE_H
#define APPS_APP_MENU_BASE_H

#include <math.h>
#include "hal/hal.h"
#include "lang/ui_strings.h"
#include "menu.h"
#include "sys/app_base.h"
#include "ui/ui_theme.h"

class AppMenuBase : public AppBase {
public:
    void onCreate() override
    {
        current_menu_ = rootMenu();
        menu_stack_top_ = 0;
        in_content_ = false;
        content_item_ = nullptr;
        resetMenuView(0);
        dirty_ = true;
    }

    void onResume() override
    {
        dirty_ = true;
    }

    void onLoop() override
    {
        handleSerialInput();

        if (updateCurveEnergy()) dirty_ = true;
        if (updateMenuSlingDisplay()) dirty_ = true;
        updateAnimation();

        if (!dirty_) {
            return;
        }

        dirty_ = false;
        HAL_BeginFrame(UITheme::Color::kBackground);
        drawLeftPanel();
        HAL_DrawFastVLine(UITheme::Menu::kDividerX, 0, HAL_Get_Screen_Height(), UITheme::Color::kPrimary);
        if (in_content_) {
            drawContent();
        } else {
            drawMenu();
        }
        HAL_Flush();
    }

    void onDestroy() override {}

    void onKnob(int delta) override
    {
        if (in_content_) {
            return;
        }

        moveSelection(-delta);
        dirty_ = true;
    }

    void onKeyShort() override
    {
        if (in_content_) {
            closeContent();
        } else {
            handleMenuEnter();
        }
    }

    void onKeyLong() override
    {
        if (in_content_) {
            closeContent();
            return;
        }

        if (!handleMenuBack()) {
            onRootBack();
        }
    }

protected:
    virtual MenuItem *rootMenu() = 0;

    virtual bool onLeafSelected(MenuItem *item)
    {
        (void)item;
        return false;
    }

    virtual void onRootBack() {}

    virtual void drawLeafContent(MenuItem *item)
    {
        if (item == nullptr) return;

        const char *title = item->title ? item->title : "";
        HAL_SetFont(u8g2_font_wqy16_t_gb2312);
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);
        HAL_DrawText(UITheme::Layout::kContentX, UITheme::Layout::kContentTitleY, title);
        HAL_DrawText(UITheme::Layout::kContentX, UITheme::Layout::kContentBodyY, UIStrings::ContentPlaceholder());
    }

    virtual const char *clockText() const
    {
        return UIStrings::MainMenuClock();
    }

    void requestRedraw()
    {
        dirty_ = true;
    }

private:
    MenuItem *current_menu_ = nullptr;
    int selected_index_ = 0;
    int menu_target_position_ = 0;
    float visual_index_ = 0.0f;
    float menu_curve_energy_ = 0.0f;
    float menu_sling_display_ = 0.0f;
    bool in_content_ = false;
    MenuItem *content_item_ = nullptr;
    unsigned long anim_ts_ = 0;
    unsigned long menu_last_input_ts_ = 0;
    unsigned long menu_curve_ts_ = 0;
    unsigned long menu_sling_ts_ = 0;
    int menu_stack_[UITheme::Menu::kStackMax] = {};
    int menu_stack_top_ = 0;
    bool dirty_ = true;

    static float clampf(float value, float min_value, float max_value)
    {
        if (value < min_value) return min_value;
        if (value > max_value) return max_value;
        return value;
    }

    static int clampi(int value, int min_value, int max_value)
    {
        if (value < min_value) return min_value;
        if (value > max_value) return max_value;
        return value;
    }

    static float smoothstep(float value)
    {
        value = clampf(value, 0.0f, 1.0f);
        return value * value * (3.0f - 2.0f * value);
    }

    uint16_t fadeColor(uint16_t base, float intensity) const
    {
        intensity = clampf(intensity, 0.0f, 1.0f);
        uint8_t r8 = (uint8_t)((((base >> 11) & 0x1F) * 255 / 31) * intensity);
        uint8_t g8 = (uint8_t)((((base >> 5) & 0x3F) * 255 / 63) * intensity);
        uint8_t b8 = (uint8_t)(((base & 0x1F) * 255 / 31) * intensity);
        return HAL_Color565(r8, g8, b8);
    }

    const char *menuTitleText() const
    {
        if (!current_menu_ || !current_menu_->title) return "";
        return current_menu_->title;
    }

    const char *menuLeftFocusText() const
    {
        if (in_content_ && content_item_ && content_item_->left_text) {
            return content_item_->left_text;
        }
        if (current_menu_ && current_menu_->child_count > 0) {
            MenuItem *item = current_menu_->children[selected_index_];
            if (item && item->left_text) return item->left_text;
        }
        return "";
    }

    int menuCount() const
    {
        return current_menu_ ? current_menu_->child_count : 0;
    }

    void resetMenuView(int index)
    {
        selected_index_ = index;
        menu_target_position_ = index;
        visual_index_ = (float)index;
        menu_curve_energy_ = 0.0f;
        menu_sling_display_ = 0.0f;

        unsigned long now = millis();
        anim_ts_ = now;
        menu_last_input_ts_ = now;
        menu_curve_ts_ = now;
        menu_sling_ts_ = now;
    }

    void moveSelection(int delta)
    {
        int count = menuCount();
        if (count <= 0) return;

        int next = clampi(menu_target_position_ + delta, 0, count - 1);
        if (next == menu_target_position_) return;

        unsigned long now = millis();
        unsigned long gap = (menu_last_input_ts_ == 0) ? 80 : (now - menu_last_input_ts_);
        menu_last_input_ts_ = now;
        if (gap < 8) gap = 8;

        float speed_factor = 80.0f / (float)gap;
        float impulse = fminf(0.55f, 0.12f * fabsf((float)delta) + 0.10f * speed_factor);
        menu_curve_energy_ = clampf(menu_curve_energy_ + impulse, 0.0f, 1.0f);

        selected_index_ = next;
        menu_target_position_ = next;
    }

    void closeContent()
    {
        if (!in_content_) return;

        in_content_ = false;
        content_item_ = nullptr;
        dirty_ = true;
        Serial.println("Close content");
    }

    bool updateCurveEnergy()
    {
        unsigned long now = millis();
        if (menu_curve_ts_ == 0) {
            menu_curve_ts_ = now;
            return false;
        }

        unsigned long elapsed = now - menu_curve_ts_;
        menu_curve_ts_ = now;
        if (elapsed == 0 || menu_curve_energy_ <= 0.0f) return false;

        float old = menu_curve_energy_;
        menu_curve_energy_ -= (float)elapsed * 0.0028f;
        if (menu_curve_energy_ < 0.0f) menu_curve_energy_ = 0.0f;
        return fabsf(old - menu_curve_energy_) > 0.01f;
    }

    float targetMenuSlingStrength() const
    {
        float pending = fabsf((float)menu_target_position_ - visual_index_);
        if (pending <= 0.015f || menu_curve_energy_ <= 0.01f) return 0.0f;

        float motion_gate = smoothstep(pending / 0.75f);
        return menu_curve_energy_ * motion_gate;
    }

    bool updateMenuSlingDisplay()
    {
        unsigned long now = millis();
        if (menu_sling_ts_ == 0) {
            menu_sling_ts_ = now;
            return false;
        }

        unsigned long elapsed = now - menu_sling_ts_;
        menu_sling_ts_ = now;
        if (elapsed == 0) return false;
        if (elapsed > 48) elapsed = 48;

        float target = targetMenuSlingStrength();
        float old = menu_sling_display_;
        float rate = (target > menu_sling_display_) ? 0.018f : 0.010f;
        float alpha = 1.0f - expf(-(float)elapsed * rate);
        menu_sling_display_ += (target - menu_sling_display_) * alpha;

        if (menu_sling_display_ < 0.004f && target <= 0.001f) menu_sling_display_ = 0.0f;
        menu_sling_display_ = clampf(menu_sling_display_, 0.0f, 1.0f);
        return fabsf(old - menu_sling_display_) > 0.002f;
    }

    float menuVerticalOffset(float offset, int spacing_y, float sling_strength) const
    {
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

    float menuCurveOffsetX(float offset) const
    {
        float distance = fabsf(offset);
        if (distance <= 0.001f) return 0.0f;

        float d = fminf(distance, 2.7f);
        return powf(d, 1.62f) * UITheme::Menu::kCurveFactor;
    }

    float menuSlingOffsetX(float offset) const
    {
        float distance = fabsf(offset);
        if (distance <= 0.05f || menu_sling_display_ <= 0.01f) return 0.0f;

        float d = fminf(distance, 3.0f);
        float speed_curve = powf(d, 1.55f) * UITheme::Menu::kCurveSpeedBoost;
        float speed_sling = powf(d, 1.12f) * UITheme::Menu::kSlingSpeedBoost;
        return -(speed_curve + speed_sling) * menu_sling_display_;
    }

    void drawMenuWheel(int count, int right_panel_x, int right_panel_w, int center_y)
    {
        const int sw = HAL_Get_Screen_Width();
        const int sh = HAL_Get_Screen_Height();
        const int box_x = right_panel_x + UITheme::Menu::kBoxPadX;
        const int box_w = right_panel_w - UITheme::Menu::kBoxPadX * 2;
        const int box_h = UITheme::Menu::kBoxH;
        const int box_y = center_y - box_h / 2;
        const int tri_x = box_x + box_w - 6;
        const int tri_size = 7;
        const int item_spacing_y = UITheme::Menu::kItemSpacingY;

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        int line_h = HAL_Get_Font_Ascent() - HAL_Get_Font_Descent();
        MenuItem *selected = current_menu_->children[selected_index_];
        const char *selected_text = selected && selected->title ? selected->title : "";
        int selected_width = HAL_Get_Text_Width(selected_text);
        if (selected_width < 12) selected_width = 12;

        int rest_center_x = right_panel_x + right_panel_w / 2;
        int speed_center_x = rest_center_x - (int)(menu_sling_display_ * UITheme::Menu::kCenterFlingX);
        int min_center_x = right_panel_x + UITheme::Menu::kCenterMinLeftGap + selected_width / 2;
        int max_center_x = sw - selected_width / 2 - 12;
        int center_x = clampi(speed_center_x, min_center_x, max_center_x);

        HAL_DrawRect(box_x, box_y, box_w, box_h, UITheme::Color::kAccent);
        HAL_FillTriangle(tri_x, center_y - tri_size, tri_x, center_y + tri_size, tri_x - tri_size - 1, center_y, UITheme::Color::kPrimary);

        int block_x = center_x - selected_width / 2 - 6 - UITheme::Menu::kScrollBarW;
        int block_min_x = box_x + 4;
        if (block_x < block_min_x) block_x = block_min_x;
        HAL_FillRect(block_x, box_y + 2, UITheme::Menu::kScrollBarW, box_h - 4, UITheme::Color::kPrimary);

        int base_idx = (int)roundf(visual_index_);
        for (int i = base_idx - UITheme::Menu::kVisibleRadius; i <= base_idx + UITheme::Menu::kVisibleRadius; ++i) {
            if (i < 0 || i >= count) continue;

            MenuItem *item = current_menu_->children[i];
            if (item == nullptr) continue;

            float offset = (float)i - visual_index_;
            float distance = fabsf(offset);
            int item_y = center_y + (int)menuVerticalOffset(offset, item_spacing_y, menu_sling_display_);
            if (item_y < -item_spacing_y || item_y > sh + item_spacing_y) continue;

            const char *text = item->title ? item->title : "";
            int text_width = HAL_Get_Text_Width(text);
            int item_x = center_x - text_width / 2 + (int)(menuCurveOffsetX(offset) + menuSlingOffsetX(offset));
            item_x = clampi(item_x, right_panel_x + 2, sw - text_width - 5);

            float intensity = 1.0f - distance * 0.40f;
            if (intensity < 0.18f) intensity = 0.18f;
            uint16_t color = (distance < 0.55f) ? UITheme::Color::kPrimary : fadeColor(UITheme::Color::kPrimary, intensity);

            HAL_SetTextColor(color, UITheme::Color::kBackground);
            HAL_DrawText(item_x, item_y - line_h / 2, text);
        }
    }

    void drawLeftPanel()
    {
        HAL_SetTextColor(UITheme::Color::kText, UITheme::Color::kBackground);

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_DrawText(UITheme::Layout::kLeftTitleX, UITheme::Layout::kLeftTitleY, menuTitleText());

        HAL_SetFont(u8g2_font_wqy12_t_gb2312);
        HAL_DrawText(UITheme::Layout::kLeftFocusX, UITheme::Layout::kLeftFocusY, menuLeftFocusText());

        HAL_SetFont(u8g2_font_freedoomr25_tn);
        HAL_DrawText(UITheme::Layout::kClockX, UITheme::Layout::kClockY, clockText());
    }

    void drawMenu()
    {
        int count = menuCount();
        if (count <= 0) return;

        const int right_panel_x = UITheme::Menu::kDividerX + 1;
        const int right_panel_w = HAL_Get_Screen_Width() - right_panel_x;
        const int center_y = HAL_Get_Screen_Height() / 2;
        drawMenuWheel(count, right_panel_x, right_panel_w, center_y);
    }

    void drawContent()
    {
        drawLeafContent(content_item_);
    }

    void updateAnimation()
    {
        if (in_content_) return;

        float target = (float)menu_target_position_;
        float diff = target - visual_index_;
        if (fabsf(diff) <= UITheme::Menu::kVisualEps) {
            if (visual_index_ != target) {
                visual_index_ = target;
                dirty_ = true;
            }
            return;
        }

        unsigned long now = millis();
        unsigned long dt = now - anim_ts_;
        if (dt < UITheme::Menu::kFrameFastMs) return;

        anim_ts_ = now;
        if (dt > 48) dt = 48;

        float base_alpha = 0.20f + fminf(menu_curve_energy_, 1.0f) * 0.07f;
        float alpha = 1.0f - powf(1.0f - base_alpha, (float)dt / 16.0f);
        visual_index_ += diff * alpha;
        if (fabsf(target - visual_index_) <= UITheme::Menu::kVisualEps) {
            visual_index_ = target;
        }
        dirty_ = true;
    }

    void handleMenuEnter()
    {
        if (!current_menu_ || menuCount() <= 0) return;

        MenuItem *sel = current_menu_->children[selected_index_];
        if (sel == nullptr) return;

        if (sel->children && sel->child_count > 0) {
            if (menu_stack_top_ < UITheme::Menu::kStackMax) {
                menu_stack_[menu_stack_top_++] = selected_index_;
            }
            current_menu_ = sel;
            resetMenuView(0);
            dirty_ = true;
            Serial.printf("Enter: %s\n", sel->title ? sel->title : "");
            return;
        }

        if (onLeafSelected(sel)) {
            dirty_ = true;
            return;
        }

        in_content_ = true;
        content_item_ = sel;
        dirty_ = true;
    }

    bool handleMenuBack()
    {
        if (!current_menu_ || !current_menu_->parent) return false;

        current_menu_ = current_menu_->parent;
        int restore_index = 0;
        if (menu_stack_top_ > 0) {
            restore_index = menu_stack_[--menu_stack_top_];
        }
        resetMenuView(restore_index);
        dirty_ = true;
        Serial.println("Back");
        return true;
    }

    void handleSerialInput()
    {
        while (Serial.available()) {
            char c = Serial.read();

            if (in_content_) {
                if (c == 'q' || c == 'Q') {
                    closeContent();
                }
                continue;
            }

            switch (c) {
                case 'w':
                case 'W':
                    moveSelection(-1);
                    dirty_ = true;
                    break;

                case 's':
                case 'S':
                    moveSelection(+1);
                    dirty_ = true;
                    break;

                case 'e':
                case 'E':
                    handleMenuEnter();
                    break;

                case 'q':
                case 'Q':
                    if (!handleMenuBack()) {
                        onRootBack();
                    }
                    break;
            }
        }
    }
};

#endif
