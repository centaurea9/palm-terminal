#ifndef SYS_FS_H
#define SYS_FS_H

#include <Arduino.h>

void SysFS_Init();
String SysFS_Read_File(const char *filepath);
bool SysFS_Write_File(const char *filepath, const char *content);

#endif
