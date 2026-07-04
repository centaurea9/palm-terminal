#ifndef SYS_APP_MANAGER_H
#define SYS_APP_MANAGER_H

#include "sys/app_base.h"
#include "sys/app_registry.h"
#include "sys/sys_constants.h"

class AppManager {
public:
    void begin();
    void run();

    void launch(AppId id);
    void push(AppId id);
    void pop();

    AppBase *currentApp() const { return current_app_; }

private:
    AppBase *current_app_ = nullptr;
    AppBase *nav_stack_[SysConst::kMenuStackMax] = {};
    int stack_top_ = 0;
};

extern AppManager appManager;

#endif
