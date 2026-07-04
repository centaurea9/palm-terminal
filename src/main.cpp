#include <Arduino.h>

#include "hal/hal.h"
#include "sys/app_manager.h"
#include "sys/sys_constants.h"

void setup()
{
    Serial.begin(SysConst::kSerialBaud);
    delay(100);
    Serial.println("\n=== Palm Terminal ===");

    HAL_Init();

    Serial.printf("Display: %dx%d\n", HAL_Get_Screen_Width(), HAL_Get_Screen_Height());
    Serial.printf("EC11 pins: A=%d, B=%d, SW=%d\n", SysConst::kEc11PinA, SysConst::kEc11PinB, SysConst::kEc11PinSw);
    Serial.println("EC11: CW=up, CCW=down, click=enter, hold=back");

    appManager.begin();
}

void loop()
{
    appManager.run();
    delay(1);
}
