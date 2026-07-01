# 掌上终端

基于 ESP32-S 的手持信息终端，支持中英文混合显示。

## 硬件

- **主控**: ESP32-S (ESP32 Dev Module)
- **显示**: ST7789 TFT LCD (240×240, 实际使用 76×284 竖屏区域，旋转为 284×76 横屏)
- **输入**: EC11 旋转编码器（已分配引脚，软件待实现）

## 功能特性

- ✅ ST7789 驱动，284×76 横屏显示
- ✅ 中英文混合渲染（TFT_eSPI 原生英文 + U8g2 中文字体）
- ✅ PWM 背光亮度可调
- ✅ WenQuanYi 12px GB2312 中文字库
- ⬜ EC11 旋转编码器输入

---

## 引脚分配

### ST7789 显示屏 (VSPI)

| ST7789 | ESP32-S | 说明 |
|--------|---------|------|
| SCL    | GPIO 18 | VSPI SCK（硬件SPI） |
| SDA    | GPIO 23 | VSPI MOSI |
| RES    | GPIO 16 | 复位 |
| DC     | GPIO 4  | 数据/命令选择 |
| CS     | GPIO 17 | 片选 |
| BLK    | GPIO 32 | 背光（PWM 可调） |
| VCC    | 3.3V    | |
| GND    | GND     | |

### EC11 旋转编码器

| EC11 | ESP32-S | 说明 |
|------|---------|------|
| A / CLK | GPIO 26 | 编码器 A 相 |
| B / DT  | GPIO 27 | 编码器 B 相 |
| SW      | GPIO 25 | 按键（按下为 LOW） |
| COM     | GND     | 公共端 |

> A/B/SW 使能内部上拉 (`INPUT_PULLUP`)，无需外接上拉电阻。

### 避开的引脚

| GPIO | 原因 |
|------|------|
| 6 ~ 11  | 内部 SPI Flash 占用 |
| 0, 2, 5, 12, 15 | 启动自举引脚（Strapping） |
| 34 ~ 39 | 仅输入，无内部上拉 |

---

## 目录结构

```
palm-terminal/
├── platformio.ini              ← PlatformIO 项目配置 + 引脚宏
├── src/
│   ├── main.cpp                ← 主程序入口
│   ├── screen/
│   │   ├── screen.h            ← 显示模块接口
│   │   └── screen.cpp          ← 显示模块实现（初始化、背光、文字渲染）
│   └── u8g2_wrapper/           ← U8g2_for_TFT_eSPI 适配层
│       ├── u8g2_fonts.h        ← 中文字体声明（文泉驿 GB2312）
│       ├── U8g2_for_TFT_eSPI.h ← U8g2 渲染适配器头文件
│       └── U8g2_for_TFT_eSPI.cpp ← U8g2 渲染适配器实现
```

### 架构

```
main.cpp          ← 应用层（页面演示）
   │
screen.h/cpp      ← 显示抽象层
   ├── TFT_eSPI            ← ST7789 硬件驱动 (SPI)
   └── U8g2_for_TFT_eSPI   ← 中文字体渲染适配器
        └── u8g2_fonts.h   ← 文泉驿字库数据 (来自 U8g2)
```

---

## 开发环境

- **PlatformIO** + **Arduino** 框架
- **依赖库**:
  - `bodmer/TFT_eSPI` (^2.5.0) — ST7789 显示驱动
  - `olikraus/U8g2` (^2.35.0) — 中文字库数据

### 常用命令

| 命令 | 说明 |
|------|------|
| `pio run` | 编译 |
| `pio run -t upload` | 编译并烧录 |
| `pio run -t monitor` | 串口监视 (115200) |
| `pio run -t upload && pio run -t monitor` | 烧录后查看输出 |

### 编译配置 (platformio.ini)

SPI 频率 20 MHz，通过 `build_flags` 传入 TFT_eSPI 的引脚和驱动配置（`USER_SETUP_LOADED=1` 跳过库默认配置）。中文字体通过 `-I src/u8g2_wrapper` 引入自定义字体桥接头文件。
