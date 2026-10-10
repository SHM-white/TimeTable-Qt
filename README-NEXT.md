## M2/M3 更新：真正恢复旧版 UIElements（2026-10-11）

本分支重新引入独立、可序列化的 **ElementDefinition/TextElement** 信息元素：它们不再被压成卡片字符串。`现代桌面卡片 → 右键 → 编辑自由元素布局`，可以通过画布拖拽设置元素位置，并编辑以下参数：

- 元素坐标与宽高；每个文字条目的内容类型、模板、字体、字号、文字颜色（含 RGBA）、粗体。
- 一个元素中可选 **单项、逐项轮播、多项同时显示**；轮播间隔可配置。
- 超出元素边界的长文字可开启平滑横向滚动；关闭时会安全省略。
- 支持 `clock/current/next/nextTime/countdown/duty/note/today/progress/week/weather` 类型。旧版时间字符串支持 C `strftime` 格式；倒计日支持 `%03d` 等数字占位符；其他数据模板支持 `{value}`。
- “今日课表”可横向或竖向排列、简称或全称，并标注当前课程。
- 每张卡片可独立设置背景色和背景图片；展开卡片可按元素占用范围自动调整尺寸。
- 导入旧版 `config.json` 时，保留 UIElements 的 **Location、Size、Data、TextFormat、TextColor、Font、TextSize、ElementType**，以及背景色/图片（如果图片文件仍然存在），而非简单转为字符串字段。未支持的原版 News / DailyWord / WeatherBlock 等元素会跳过并计数。旧项目从未完整实现它们，不能把跳过当作完整迁移。

### 天气配置

天气采用可选的高德 Web API，元素编辑器提供 API Key、Adcode（如 110000）与城市备注，天气请求通过 QtNetwork 异步发送并缓存约一小时。没有 Key 时不会联网。**API Key 当前与布局一起保存在本机 QSettings 中（非加密），请勿把含密钥的配置导出到公开仓库**。Win7 下 HTTPS/系统加密服务的兼容性需要实机测试，故不会承诺 Win7 一定能获取天气。

### 构建与测试

`timetable_elements` 是 Qt5/Qt6 共享的元素渲染模块；`tests/element_tests.cpp` 回归测试旧版元素几何、样式、轮播、倒计时格式和背景独立存储。两套版本保留独立 Windows 便携包，Win7 版本仅作为兼容候选。

限制：目前已实现组件级精确坐标的可视化拖拽、属性编辑，**尚未实现**画布吸附对齐、分组多选、完整旧版所有实验性元素。WinUI 风格设置窗口继续使用 Qt Widgets，不引入 Windows App SDK，从而不破坏 Qt5/Win7 兼容性。

---

## Qt 5 / Windows 7 兼容候选与设置页更新

当前分支的 CMake 支持 Qt6 主构建与 Qt5.15 兼容构建。Windows 系统的设置管理页改为更接近 WinUI Settings 的导航与分组卡片设计，实际基于 Qt Widgets（不是原生 Windows App SDK，也尚未绑定第三方 Fluent 库）。

```sh
cmake -S . -B build-qt5 -DTIMETABLE_FORCE_QT5=ON -DBUILD_TESTING=ON
cmake --build build-qt5 --config Release
ctest --test-dir build-qt5 --output-on-failure
```

CI 另有 Windows Qt5.15.2 便携包 `TimeTableNext-Qt5-Win7-Candidate`。**这是 Win7 候选包，而非经过 Win7 实机认证的发行版**；需在 Windows 7 SP1 虚拟机或真实系统验证 VC Runtime、Qt qwindows 平台插件、缩放、多窗口和托盘。Win11/10 主版本继续使用 Qt6。

本轮同步修复了旧课表七天 `Infos` 附加文本的导入与往返保存，旧 MainWindow 也纳入窗口导入器。完整历史功能差异请看 `docs/LEGACY_FEATURE_AUDIT.md`。

---

## 当前版本：紧凑卡片与原版功能迁移（本次更新）

**第一次运行即显示精简卡片**：时钟约 158×48，当前课程/下一节课程约 195×54，而不再默认创建三块约 310×150 的大窗口。已有上个预览版配置的窗口也默认迁移成小尺寸，并保留屏幕位置。

- 右键任一卡片 → **折叠成小卡片 / 展开卡片**。折叠只显示一行关键文字，展开则显示详细内容。
- 用鼠标拖动卡片空白位置移动；**拖右下角的斜线手柄自由缩放**。折叠、展开各自记住宽高，最小分别为 125×44 和 190×102。
- 右键 → **设置此窗口**，配置标题、文字大小、背景透明度、紧凑卡片是否显示标题、折叠/展开两套尺寸、信息类型、置顶与位置锁定。
- **组合信息栏**支持 `clock,date,current,next,week,duty,countdown,note` 任意排列。可选择全部显示，或按 2–300 秒间隔逐项轮播，对应旧版的「多个元素按顺序显示」。
- 新增 **每日信息/值日**：右键或桌面窗口管理页可编辑每周七天的文本，组合信息也能引用当天值日。
- **开机自启动（Windows）**：桌面窗口管理页开关，使用 HKCU Run；不需要管理员权限。
- **旧版窗口迁移**：桌面窗口管理页 → 「导入旧版窗口」，选择历史 `config.json`，尝试把 Windows/UIElements（时钟、课程、倒计日、值日等）转换为紧凑组合卡片。只追加窗口，原有课表、原有文件不被覆盖；不能识别的旧类型会在导入前统计跳过个数。
- **一键隐藏/恢复**：右键任一卡片可隐藏全部；系统托盘或桌面窗口管理页随时恢复，不会被挡屏。
- **切页残影修复**：管理器不再给整个 QStackedWidget 页面加 QGraphicsOpacityEffect，切换页面只交给 Qt 正常管理可见性；独立卡片首次出现的轻量透明度动画仍在。

尚未迁移的旧功能：高德天气（依赖外部 API Key）、新闻/每日一句、旧版完整自由像素布局 UIElements 以及复杂的多数据块渲染。这些不会用假数据冒充实现。Windows 真实 DPI、多屏、透明窗口行为仍需真人桌面验收。

**升级说明：**布局存在用户 QSettings 中，与旧版课表 JSON 独立。新版不会修改源文件。若当前卡片尺寸不合适，可以在「管理桌面信息窗」逐一调整折叠与展开的尺寸。

---

# TimeTable Next · 多窗口桌面工具重构

> **方向调整：默认不是大型课表窗口**。程序启动后直接显示独立的常驻桌面信息卡片，完整课表管理器仅在需要时打开。这次迭代恢复了原项目「多窗口、置顶显示、按层级展开信息」的设计意图。

## 桌面上的默认布局

首次启动显示 **时间与日期、当前课程、下一节课程** 3 个独立窗口。左键点击卡片会弹出该信息的详情，双击进入完整课表；右键提供配置、增加/删除窗口、锁定位置、置顶、隐藏和退出等菜单。窗口可以拖动，坐标、大小、信息类型和布局会持久化。

右键任意小窗 → **管理全部桌面窗口**，即可添加、编辑、隐藏或删除卡片；即使全部隐藏，也能通过系统托盘恢复。

目前支持的可配置窗口类型：时钟与日期、当前课程、下一节课程、今日课程、教学周信息、目标日倒计时、自定义便签。每个窗口单独设置标题、配色（柔和浅色 / 夜间深色）、不透明度、文字大小、宽高、置顶、锁定，以及是否单击弹出详情。

**深色模式兼容：** 程序使用 Fusion 控件风格和完整的 Qt 调色板，不从 Windows 暗色系统主题不受控地继承文字颜色。即使系统设成深色，应用选中浅色时也应保证文字对比；小窗可以独立选择主题。切换深浅主题不依赖系统设置。

**完整课表管理器** 仍可通过小窗右键、双击、详情弹层中的按钮、托盘菜单打开；里面保留教学周/单双周/CSV/JSON/ICS/调休设置等复杂功能，但不再强制启动就占据整个桌面。

## 已做与尚未完成

- 已实现：多独立窗口、逐窗口配置、托盘管理与布局持久化、单击弹层、课程业务规则、主题显式设置、低频内容刷新与每秒倒计时更新。
- CI 覆盖：Linux 课程测试、无显示器环境下浅色/深色窗口启动测试、Windows Qt6 编译与便携 ZIP。
- 尚待真实 Windows 检验：高 DPI 下窗口位置、系统深色模式/显示缩放、托盘行为、透明窗口渲染和动画观感。
- 后续：图形化拖拽布局编辑器、同一窗口内嵌多个可排列信息块、桌面边缘吸附、交互式快速调休、系统通知、完整旧版 UIElements 模型迁移。

---
# TimeTable Next · 新版校园桌面课表

> 现代 Qt6/CMake 预览版。旧版源代码仍在 `TimeTable-Qt/`，新工程独立存在，避免重构损坏旧版。

## 已实现

- **教学周**：学期开学日期、总周数、单/双周与离散指定周。
- **调休**：单日休息、周末补课按照指定日期的课程表显示（含教学周规则）。
- **直观编辑**：课程增删改复制、时间/教师/地点/颜色设置、节次时间表。
- **桌面交互**：今日概览、课程进度/下课倒计时、七日周视图、深浅主题、淡入过渡、系统托盘。
- **导入导出**：新版 JSON、旧版 Lessons.json、UTF-8 CSV、小爱 courseInfos 格式的本地 JSON；导出 JSON、CSV、ICS 日历和小爱结构 JSON。
- **离线优先**：本地用户数据目录持久化，`QSaveFile` 原子写入，无网络或登录依赖。

## 下载 Windows 二进制

在本分支的 GitHub Actions 中找到 **TimeTable Next - Build and Package**，选择成功的 Windows x64 作业，下载 `TimeTableNext-Windows-x64` artifact，解压后运行 `TimeTableNext.exe`。打包流程自动运行 `windeployqt`，无需额外安装 Qt Runtime。

**注意：**只有 CI 构建通过且 artifact 确认存在后，才能视为已提供经过自动构建验证的 Windows 包。请以对应运行状态为准。

## 本地编译

Qt 6.4+、CMake 3.21+、C++17：

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Windows 选用 Qt Creator 或 MSVC 编译，运行 `windeployqt` 收集库后分发。Linux 可安装发行版的 `qt6-base-dev`。

## 输入 CSV 格式

使用 UTF-8（可含 BOM），列为 name,weekday,start,end,weeks,teacher,room：

```csv
name,weekday,start,end,weeks,teacher,room
离散数学,1,08:00,09:40,"1-16单",王老师,教一 201
数据库,3,14:00,15:40,"2-18双",李老师,机房 402
```

周一=1 至周日=7。周数支持 `全部`、`1-16`、`1-16单`、`2-18双`、`1-8,10,12`。

也支持旧版 CSV：`Mon,课程名称,0800,0940`。首次启动附带 3 门可删除的示例课程。**导入默认替换当前课表，请先导出 JSON 备份。**

## 小爱课程表范围

已支持从含有 `courseInfos` 数据数组的 JSON 中解析 name、day、sections、weeks、teacher 和 position；也支持生成该结构的 JSON。导入前请在设置中核对节次与时间的映射。

这不是小爱官方的通用文件导入/导出协议，不代表该 JSON 一定能直接导回手机 App。程序不登录小爱账户，不调用不公开的 API，不上传 cookie/token。官方能否接受文件需要实际设备验证。

## 学期示例

当 semesterStart 为 2026-09-07，某课程只在第 1、3、5 周的周一上课：9 月 7 日有课、14 日没有课。将 9 月 12 日设为按 9 月 7 日上课，则 9 月 12 日也有该课程；将某日设为“休息日”则当天不显示课程。

数据位置是系统 `QStandardPaths::AppDataLocation` 下的 `schedule.json`，支持直接导出和备份。

## 技术结构

- `modern/ScheduleStore.*`：业务领域模型、教学周、调休、序列化、各格式互通。
- `modern/MainWindow.*`：面板、课程编辑、设置、过渡动画与系统托盘。
- `tests/schedule_tests.cpp`：排课、边界、调休与文件格式回归。
- `.github/workflows/modern-build.yml`：Linux 测试 / Windows 便携 ZIP。

## 后续待办

- Windows 桌面、高 DPI、托盘行为的人机测试。
- 大学教务导入模板、课程冲突提示、多课表与多学期。
- 桌面悬浮课表 / 系统通知 / 快捷键 / 搜索及高级筛选。
- 原版自由布局多窗口、天气、倒数日与开机自启的迁移（**当前未迁移**）。
- 小爱样例脱敏兼容性测试，可能探索 WakeUp 备份格式作为中转。
- CSV 含换行字段与 ICS 显式时区的进一步兼容。

参见 `docs/REFACTOR_REVIEW.md`，包含老项目代码审查和各项重构情况。
