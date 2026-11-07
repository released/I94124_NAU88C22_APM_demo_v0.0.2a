# 第三方組件

| 組件 | 位置與用途 | 原始聲明 |
|---|---|---|
| Nuvoton I94100 BSP | `Library/` 及 USB 範例的 `Library/`，提供 peripheral driver、device header 與啟動支援 | 原始檔保留 Nuvoton copyright 聲明 |
| Arm CMSIS | 兩份 SDK 的 `Library/CMSIS/`，提供 Cortex-M header 與 DSP 支援 | 各檔案依原始聲明；例如 [core_cm4.h](Library/CMSIS/Include/core_cm4.h) 標示 `Apache-2.0` |
| Colsonic APM | 各範例的 `Colsonic/`，包含封閉 library 與參數 profile | 此 sample package 未附獨立的 Colsonic redistribution license 文件 |

- 各組件的原始 copyright、license 與其他 notices 保留於對應檔案。
- 本 repository 未對上述第三方組件另行授予授權。
- Colsonic library、profile 與配套配置需成套使用，詳見根目錄 README。
