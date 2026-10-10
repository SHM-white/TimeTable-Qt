# 旧版功能完整盘点与下一阶段迁移方案

审计基线：原始 `master` 分支（2026-10-11），对照 `refactor/modern-campus-desktop`。本文件基于 **源码行为**，不是只看 README 或仅仅统计枚举值。

## 关键结论

旧版的产品核心不是“大课表窗口”，而是 **BasicWindow 容器 + UIElementBase 信息元素 + Settings_New 配置编辑器**。此前新版的卡片是基于固定 `type` 单组件设计，虽然现在补充了组合信息和轮播，但依旧不能完整表达任意元素几何、字体、颜色、排版、数据配置。后续应抽象成可序列化的 `WindowDefinition` / `ElementDefinition`，由同一渲染器为 Qt5 与 Qt6 提供桌面窗口。

## 真实功能覆盖矩阵

| 功能与源代码 | 原版实现状况 | 当前新版情况 | 下一步 |
|---|---|---|---|
| 多窗口 `BasicWindow.cpp`、`TimeTableQt.cpp` | 已实现，窗口分别有位置、大小、显示、置顶、可拖动 | 已迁移且增加折叠、大小保存 | 保留数据和性能边界 |
| UIElements 自定义 `Location` / `Size` | 已实现，`QPainter` 按元素区域绘制 | **缺失元素级自定义** | 新增可视化元素画布 |
| 单项文字 `SingleTextItem` | 已实现，每个元素独立字体、颜色（RGBA）、字号、文本模板 | **缺失独立属性** | 建立统一元素样式属性面板 |
| 多项轮换 `MultiTextItem` | 已实现，按条件切换文本 | 只有按间隔轮播 | 恢复基于内容长度和事件的切换规则 |
| 超长文字的滚动动画 `SingleTextItem::paint` | 已实现，使用 `ITEM_SCROLL_SPEED` | **缺失** | 限制刷新率，基于 QElapsedTimer 平滑滚动 |
| 窗口自动适应内容 `BasicWindow::updateWindowStatus` | 已实现，按元素范围推断窗口大小 | **缺失** | 支持固定尺寸 / 内容适配两种模式 |
| 透明色/背景图片 `BasicWindow` | 已实现，可设置 `BackGroundColor`/图像 | **缺失背景图** | 恢复图片与 RGBA、按内容遮盖 |
| 高精度秒表/时钟 `CurrentTime` | 已实现，自定义 strftime 模板 | 有时钟但格式固定 | 加入时间格式编辑与即时预览 |
| 当前课程、下一节课、下节倒计时 | 已实现 | 已迁移，但样式固定 | 元素可插入任意容器 |
| 倒数日 `CountDownDay` | 已实现，秒级与格式字符串 | 只有简化倒计日 | 自定义格式与多个目标 |
| 每日附加消息 `Infos` / `AddMoreInfo` | 已实现，存于课程 JSON 的七天条目中 | 已补齐 Infos 导入、JSON 往返及每日卡片编辑；尚缺元素级排版 | 将原 Infos 纳入新版数据模型与导入导出 |
| 单日完整课表 `TodayAllLessons` | 有绘制类；README 表示 v3 后砍掉独立窗口 | 有弹层/主视图 | 恢复可选紧凑桌面课程列表 |
| 天气 `Timetable.cpp::GetWeather` | 使用高德天气 API，请求/解析逻辑存在 | **缺失** | QtNetwork 重写为异步服务，用户自带 API Key |
| 新闻 `News`、每日一句 `DailyWord` | 枚举存在；`SingleTextItem` 分支落入固定文本 | 原版未完成，新版也未实现 | 可选插件，不列为已迁移 |
| `MultiItemAllDisplay` | 类存在，但 `BasicWindow` 分支返回占位元素 | 未实现 | 新版组件系统支持网格/横竖排列 |
| `CurrentLessonProgress` / `Clock` / `WeatherBlock` | 枚举存在，但绘制实现/工厂主要为占位或异常 | 主界面倒计时/进度有部分功能 | 实现真正的小组件，不能以枚举当作历史完成项 |
| 窗口 FPS、调试 FPS、定时刷新 | 已实现但部分低效 | 定时刷新，未开放可配刷新策略 | 数据驱动刷新 + 动画帧率限制 |
| 教学周、单双周、调休 | 原版不具备统一模型 | 新版新增 | 保持并增强 |
| CSV 导入，旧 `Lessons.json` | 已实现 | 已实现 | 进一步修复时间格式与附加消息 |
| 系统托盘、注册表开机自启 | 已实现 | 已实现 | Win7/Win11 真机冒烟 |
| 亚克力 `AcrylicEffect` | 实验功能且已禁用，Win11 效果不稳定 | 未迁移 | Win11 专属可选增强，Win7 关闭 |
| 界面语言/翻译文件 `*_zh_CN.ts` | 文件存在但翻译内容很少 | 无完善 i18n | 不算“丢失的成熟功能” |
| Windows 安装程序 `*.iss` | 有 Inno Setup 脚本 | 只有便携 ZIP | 补安装版，可选择开机启动 |
| 配置路径迁移与切换 | 已有配置路径变更 UI | 新版默认用户目录/导入导出 | 提供完整的配置迁移与回滚向导 |

## 设置界面视觉和交互规划

设计目标是 **Windows Settings / WinUI 3 的视觉与交互方式**，而不是把真正的 Windows App SDK 编进 Qt 程序。主程序必须与 Qt5/Qt6 共用 QWidget 基础。

1. 顶部显示当前页面名称、明确的作用说明；不要长篇欢迎语，也不要使界面看上去像网页管理后台。
2. 左侧 NavigationView 风格窄导航；清晰的选中强调条；只存在一种统一圆角半径（设置项建议 8–10 px）。
3. 设置项为“标题 + 说明 + 右侧控件”，按“学期、外观、桌面组件、节次、数据”分组，控件有完整 hover/focus/disabled 状态。
4. 日历和课程管理使用专门的表格/编辑器；桌面组件属性与课程数据彼此分离，不应出现在一个超大表单。
5. 动画仅用于局部状态变化，不对整个 QWidget 或含原生表格的页面施加 QGraphicsOpacityEffect，避免 Windows 切页残影。
6. 支持深浅两套完整 QPalette/QSS 和 100%/150%/200% DPI；Win7 不依赖 Mica、DWM Acrylic、新系统图标字体。

### Qt C++ Fluent 第三方评估（2026-10-11）

- **优先评估 [Type3limit/QtFluentWidgets](https://github.com/Type3limit/QtFluentWidgets)**：Qt5/Qt6、Qt Widgets、MIT；组件与主题体系较完整，附带 Designer/Demo 和 Lottie。适合原 MIT 仓库。仍需锁定 commit，单独检查 rlottie/字体/编译配置和 Win7 运行兼容。
- **备选 [toddming/QFluentKit](https://github.com/toddming/QFluentKit)**：Qt5/Qt6、丰富控件，但许可证 GPL-3.0；不建议未经整体许可证评估直接引入 MIT 项目。
- **官方同名 QFluentWidgets C++**：并非开源免费，属于独立的收费库；其 Python 代码不是可直接用于 C++ 项目的 Qt Widgets 实现。
- 为了先保证双版本可用，目前以原生 Qt Widgets 构建了 WinUI 风格导航、分组设置行、完整浅深主题；**尚未实际引入第三方 Fluent 依赖**。

## Qt 5 / Windows 7 兼容策略

- 同一源码、同一业务模型：CMake `-DTIMETABLE_FORCE_QT5=ON` 编译 Qt5.15，Qt6 为现代默认。
- 使用 `QtCompat.h` 屏蔽 `QMouseEvent::position/globalPosition` 和 `QFont::setFamilies` 等 Qt6 特有 API。
- Linux Qt5 + Qt6 的 CTest 与离屏 UI smoke 均为强制 CI。
- Windows Qt6 MSVC + Qt runtime 发布现代 ZIP。Qt5 另有 Windows 便携版候选 CI，**不能只凭 Windows10/11 CI 就承诺 Win7 支持**。
- 完整 Win7 SP1 发行仍需目标系统检查：`qwindows.dll`、VC++ 运行库、可执行文件依赖 API、显示缩放、多窗口拖动、托盘、开机启动；若缺少实际 Win7 VM 测试，则明确标记为候选包。
- Windows7 系统已不再获得常规系统安全更新，应保持完全本地可用，天气 API/网络请求属于可选能力。

## 推荐分期

- **M1 稳定和兼容**：完成 Qt5/Qt6 编译，修复切页/主题，验证两条便携 ZIP 流水线。
- **M2 内容引擎**：引入 `ElementDefinition`（type、format、font、RGBA、rect、behavior）和 `WindowDefinition`，为旧配置迁移与可视化编辑准备。
- **M3 旧功能回归**：自由定位、文本滚动、多源轮播、背景图片、自动大小、每日 Infos 数据。
- **M4 增值和系统能力**：天气异步服务、通知、独立今日课表、可选 Fluent 组件库、安装包、实际 Windows7 兼容测试。

## 附：迁移中已发现的数据缺口

旧版 `Lessons.json` 每天有 `Lessons` 和 `Infos` 两项。审计时发现新版原本遗漏 `Infos`；本轮已在 `ScheduleStore` 补齐 `dayInfos` 的导入、存储及测试，旧占位值会被过滤。今后迁移其它格式仍应坚持临时模型校验，避免静默丢数据。
