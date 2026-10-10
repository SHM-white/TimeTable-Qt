# TimeTable Next v4.1.0

首个重新设计的多窗口桌面课表版本，保留旧源码并新增跨 Qt5/Qt6 的共用核心。

### 主要功能

- 桌面独立可置顶小窗，支持折叠、调整尺寸、背景、深浅主题、轮播以及拖拽布局
- UIElements 元素级坐标、RGBA 字体、时间格式、文字滚动、旧版 config.json 元素导入
- 大学课程单双周/持续周、调休与补课、课程编辑、当前/下一节课程、今日课表
- CSV、JSON、ICS 互通；小爱课程表 courseInfos 结构转换（**不代表官方 App 直接导入**）
- 高德天气可选启用（API Key 由用户自行填写，保存在本机）
- Qt6 Windows x64、Qt5 Windows x64 兼容候选、Linux Qt6/Qt5 x86_64 动态运行时版本

### 选择下载文件

| 平台 | Release 文件 | 注意事项 |
|---|---|---|
| Windows 10 / 11 x64 | `TimeTableNext-v4.1.0-Windows-Qt6-x64.zip` | 推荐解压运行 |
| Windows 7 SP1 / 8 / 10 / 11 x64 | `TimeTableNext-v4.1.0-Windows-Qt5-x64-Win7-Candidate.zip` | **Win7 未实机认证**，需要可用的 VC++ 运行时 |
| Ubuntu 24.04 x86_64 + Qt6 | `TimeTableNext-v4.1.0-Linux-Qt6-amd64.deb` | `sudo apt install ./文件名.deb` |
| Ubuntu 24.04 x86_64 + Qt5 | `TimeTableNext-v4.1.0-Linux-Qt5-amd64.deb` | Qt5 兼容替代 |
| 其他 x86_64 Linux | `Linux-Qt6-x86_64.tar.gz` 或 `Linux-Qt5-x86_64.tar.gz` | **动态链接**，须另行安装相应的 Qt 和 C/C++ 运行库；不保证所有发行版均兼容 |

Linux Qt5 和 Qt6 的 .deb 包包名相同，不可同时安装；tar.gz 可分别解压运行。macOS 暂未建立经过构建验证的发布目标，因此不在此次“受支持平台”清单内。

### 已知限制及注意事项

- **Windows 7**：仅 Windows CI 上使用 Qt 5.15 构建和测试，未在 Win7 SP1 真机 / 虚拟机验证 DLL、TLS、DPI 和托盘。
- **Linux**：从 Ubuntu 24.04 编译，其他发行版的 glibc / Qt 库版本可能不兼容，tar.gz 不包含 Qt 运行库。
- 新闻、每日一句、部分早期实验组件尚未迁移；自由元素编辑尚缺多选/吸附/分组。
- 启用天气的 API Key 当前按用户配置明文存储，不要对外分享配置文件。
- 导入课程将替换原课表，请先导出 JSON 备份；旧窗口的配置导入是追加操作。

参见 [README-NEXT.md](README-NEXT.md) 和 [功能差异审计](docs/LEGACY_FEATURE_AUDIT.md)。
