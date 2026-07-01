# 掌上终端

## 硬件

- **主控**: ESP32-S
- **显示**: ST7789 TFT LCD (240×240, SPI)
- **输入**: EC11 旋转编码器

## 代码路径

根目录 → 模块文件夹 → 模块代码

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
| BLK    | GPIO 32 | 背光（PWM可调） |
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
掌上终端/                   ← PlatformIO 项目根目录
├── platformio.ini          ← 全局编译配置 + 引脚宏
├── src/
│   └── main.cpp            ← 唯一入口
├── 屏幕模块/                ← ST7789 驱动 (screen.h/cpp)
└── （后续模块...）
```

> `main.cpp` 在根目录 `src/` 下，不在子模块里。子模块只放对应驱动的 `.h/.cpp`。

## 开发环境

- **PlatformIO** + **Arduino** 框架
- 编译 & 烧录：`pio run -t upload`
- 串口监视：`pio run -t monitor`
