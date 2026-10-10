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
