#ifndef INPUT_EC11_H
#define INPUT_EC11_H

#include <Arduino.h>

struct EC11Config {
    static constexpr uint8_t NO_PIN = 0xFF;

    uint8_t pin_a = NO_PIN;
    uint8_t pin_b = NO_PIN;
    uint8_t pin_button = NO_PIN;
    bool invert_rotation = false;
    uint16_t button_debounce_ms = 25;
    uint16_t long_press_ms = 650;
};

class EC11Input {
public:
    void begin(const EC11Config &config);
    void update();

    int consume_steps();
    bool consume_click();
    bool consume_long_press();

private:
    uint8_t read_encoder_state() const;
    bool read_button_pressed() const;

    EC11Config config_{};
    bool ready_ = false;

    uint8_t last_encoder_state_ = 0;
    int8_t quarter_steps_ = 0;
    int pending_steps_ = 0;

    bool raw_button_pressed_ = false;
    bool stable_button_pressed_ = false;
    bool long_press_reported_ = false;
    bool click_pending_ = false;
    bool long_press_pending_ = false;
    unsigned long button_change_ms_ = 0;
    unsigned long button_press_ms_ = 0;
};

#endif
