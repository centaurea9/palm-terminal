#ifndef SYS_APP_BASE_H
#define SYS_APP_BASE_H

class AppBase {
public:
    virtual ~AppBase() = default;

    virtual void onCreate() = 0;
    virtual void onResume() {}
    virtual void onBackground() {}
    virtual void onLoop() = 0;
    virtual void onDestroy() = 0;

    virtual void onKnob(int delta) = 0;
    virtual void onKeyShort() {}
    virtual void onKeyLong() {}
};

#endif
