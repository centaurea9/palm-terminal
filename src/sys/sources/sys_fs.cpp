#include "sys/sys_fs.h"

#include <LittleFS.h>

namespace {

bool g_fs_ready = false;

} // namespace

void SysFS_Init()
{
    g_fs_ready = LittleFS.begin(true);
    if (g_fs_ready) {
        Serial.println("[FS] LittleFS mounted.");
    } else {
        Serial.println("[FS] LittleFS mount failed.");
    }
}

String SysFS_Read_File(const char *filepath)
{
    if (!g_fs_ready || filepath == nullptr) {
        return "";
    }

    File file = LittleFS.open(filepath, "r");
    if (!file) {
        return "";
    }

    String content = file.readString();
    file.close();
    return content;
}

bool SysFS_Write_File(const char *filepath, const char *content)
{
    if (!g_fs_ready || filepath == nullptr || content == nullptr) {
        return false;
    }

    File file = LittleFS.open(filepath, "w");
    if (!file) {
        Serial.printf("[FS] Open failed for write: %s\n", filepath);
        return false;
    }

    size_t written = file.print(content);
    file.close();
    return written == strlen(content);
}
