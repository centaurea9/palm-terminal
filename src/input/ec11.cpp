#include "input/ec11.h"

namespace {

constexpr int8_t kTransitionTable[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0,
};

constexpr int8_t kQuarterStepsPerDetent = 4;

} // namespace

EC11Input *EC11Input::instance_ = nullptr;

void EC11Input::begin(const EC11Config &config) {
    config_ = config;
    ready_ = false;

    if (config_.pin_a == EC11Config::NO_PIN || config_.pin_b == EC11Config::NO_PIN) {
        return;
    }

    pinMode(config_.pin_a, INPUT_PULLUP);
    pinMode(config_.pin_b, INPUT_PULLUP);

    if (config_.pin_button != EC11Config::NO_PIN) {
        pinMode(config_.pin_button, INPUT_PULLUP);
    }

    last_encoder_state_ = read_encoder_state();
    quarter_steps_ = 0;
    pending_steps_ = 0;

    raw_button_pressed_ = read_button_pressed();
    stable_button_pressed_ = raw_button_pressed_;
    long_press_reported_ = false;
    click_pending_ = false;
    long_press_pending_ = false;
    button_change_ms_ = millis();
    button_press_ms_ = stable_button_pressed_ ? millis() : 0;

    instance_ = this;
    attachInterrupt(digitalPinToInterrupt(config_.pin_a), handle_encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(config_.pin_b), handle_encoder_isr, CHANGE);

    ready_ = true;
}

void EC11Input::update() {
    if (!ready_) return;

    if (config_.pin_button == EC11Config::NO_PIN) return;

    unsigned long now = millis();
    bool raw_pressed = read_button_pressed();

    if (raw_pressed != raw_button_pressed_) {
        raw_button_pressed_ = raw_pressed;
        button_change_ms_ = now;
    }

    if ((now - button_change_ms_) >= config_.button_debounce_ms &&
        raw_pressed != stable_button_pressed_) {
        stable_button_pressed_ = raw_pressed;

        if (stable_button_pressed_) {
            button_press_ms_ = now;
            long_press_reported_ = false;
        } else if (!long_press_reported_) {
            click_pending_ = true;
        }
    }

    if (stable_button_pressed_ &&
        !long_press_reported_ &&
        (now - button_press_ms_) >= config_.long_press_ms) {
        long_press_pending_ = true;
        long_press_reported_ = true;
    }
}

int EC11Input::consume_steps() {
    noInterrupts();
    int steps = pending_steps_;
    pending_steps_ = 0;
    interrupts();

    if (config_.invert_rotation) {
        steps = -steps;
    }

    return steps;
}

bool EC11Input::consume_click() {
    bool pending = click_pending_;
    click_pending_ = false;
    return pending;
}

bool EC11Input::consume_long_press() {
    bool pending = long_press_pending_;
    long_press_pending_ = false;
    return pending;
}

void IRAM_ATTR EC11Input::handle_encoder_isr() {
    if (instance_ != nullptr) {
        instance_->update_encoder_from_isr();
    }
}

void IRAM_ATTR EC11Input::update_encoder_from_isr() {
    uint8_t state = read_encoder_state();
    if (state == last_encoder_state_) return;

    uint8_t transition = (last_encoder_state_ << 2) | state;
    quarter_steps_ += kTransitionTable[transition];
    last_encoder_state_ = state;

    if (quarter_steps_ >= kQuarterStepsPerDetent) {
        pending_steps_++;
        quarter_steps_ = 0;
    } else if (quarter_steps_ <= -kQuarterStepsPerDetent) {
        pending_steps_--;
        quarter_steps_ = 0;
    }
}

uint8_t IRAM_ATTR EC11Input::read_encoder_state() const {
    uint8_t state = 0;
    if (digitalRead(config_.pin_a)) state |= 0x02;
    if (digitalRead(config_.pin_b)) state |= 0x01;
    return state;
}

bool EC11Input::read_button_pressed() const {
    if (config_.pin_button == EC11Config::NO_PIN) {
        return false;
    }

    return digitalRead(config_.pin_button) == LOW;
}
