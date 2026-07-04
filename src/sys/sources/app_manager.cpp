#include "sys/app_manager.h"

#include "hal/hal.h"

AppManager appManager;

void AppManager::begin()
{
    launch(AppId::Standby);
}

void AppManager::run()
{
    if (current_app_ == nullptr) {
        return;
    }

    HAL_PollInput();

    int knob_delta = HAL_Get_Knob_Delta();
    if (knob_delta != 0) {
        current_app_->onKnob(knob_delta);
    }

    if (HAL_Consume_Click()) {
        current_app_->onKeyShort();
    }

    if (HAL_Consume_LongPress()) {
        current_app_->onKeyLong();
    }

    current_app_->onLoop();
}

void AppManager::launch(AppId id)
{
    AppBase *next = AppRegistry_Get(id);
    if (next == nullptr) {
        return;
    }

    stack_top_ = 0;
    if (current_app_ != nullptr) {
        current_app_->onDestroy();
    }

    current_app_ = next;
    current_app_->onCreate();
    current_app_->onResume();
}

void AppManager::push(AppId id)
{
    AppBase *next = AppRegistry_Get(id);
    if (next == nullptr) {
        return;
    }

    if (current_app_ != nullptr && stack_top_ < SysConst::kMenuStackMax) {
        nav_stack_[stack_top_++] = current_app_;
        current_app_->onBackground();
    }

    current_app_ = next;
    current_app_->onCreate();
    current_app_->onResume();
}

void AppManager::pop()
{
    if (current_app_ != nullptr) {
        current_app_->onDestroy();
    }

    if (stack_top_ > 0) {
        current_app_ = nav_stack_[--stack_top_];
        current_app_->onResume();
        return;
    }

    launch(AppId::MainMenu);
}
