# 旧版功能迁移审计

审计日期：2026-10-11。对照重构前 `TimeTable-Qt/` 与当前 `modern/` 的实际入口、数据模型、界面操作和测试。旧枚举、空槽函数、历史 README 及计划清单不能单独证明功能曾经可用。本文件替代先前“未迁移”矩阵；`docs/REFACTOR_REVIEW.md` 保留历史记录，其中部分待办已经完成。

本轮已迁移上次列出的天气预报/地址解析、滚完再轮播与可调速度/FPS、每日信息逐条轮换、自定义路径与布局备份、今日课表动态尺寸、旧几何/背景兼容、追加导入、临时隐藏语义以及 Windows 安装打包流程。另实现多项同时排布、新闻/每日一句数据源、图形块、编辑器多选吸附分组、提醒、学期档案、搜索冲突和中英界面。**实现存在不代表真实 Windows 效果、安装包或线上服务已经验收，也不代表所有历史计划都已完成。**

## 已迁移的成熟旧功能

| 能力 | 重构前依据 | 当前实现与用户入口 |
| --- | --- | --- |
| 多窗口、拖动、几何持久化、置顶、显示/隐藏 | `BasicWindow.cpp` 的构造、几何保存、鼠标事件；旧 `mainwindow.cpp` 的显示/隐藏入口 | `DesktopOverlays.cpp` 的 `OverlayCard`、`moved`、`showAll/hideAll`，新增折叠/展开两套尺寸、自由缩放、锁定。右键“管理全部桌面窗口”；打开管理界面时卡片暂时避让 |
| 启动 AutoOpen 与临时隐藏分离 | 旧 `BasicWindow` 区分 AutoOpen 和运行中的 `m_hide` | `shouldShow`、`setStartupVisible`、临时隐藏集合；“启动显示 / 隐藏”持久化，全部/单窗隐藏和恢复只影响当前会话 |
| 自由元素坐标、字体、字号、RGBA、模板 | 旧 `UIElementBase.cpp`、`SingleTextItem.cpp` 读取 Location/Size、TextFormat、TextColor、Font、TextSize | `ElementLayout::fromLegacy` 保留字段；“编辑自由元素布局”提供真实拖拽及属性编辑，绘制模型与预览共用 |
| 单项及多项按序显示，长文字滚完才切换 | 旧 `MultiTextItem::paint` 检查 `m_CanChange`；`SingleTextItem::paint` 在滚完或短文停留后放行 | `cycleState` 支持固定间隔及 `cycleAfterScroll`；旧导入默认滚完再切换、短文停留 5 秒 |
| 可调滚动速度与逐窗口 FPS | 旧 ItemScrollSpeed、`BasicWindow` FPS、Settings_New 对应配置项 | 元素编辑器“滚动速度”以 px/s 设置，导入按 `1000 / ItemScrollSpeed` 转换；窗口设置 FPS 为 0–120，导入旧 FPS。0 停止连续动画，业务更新/轮播仍继续 |
| 调试边框及实时绘制 FPS | 旧 `BasicWindow::paintEvent` 的 Debug、ShowFPS | “设置此窗口”的调试边框、实际绘制 FPS；静态窗口不为显示 FPS 额外持续绘制 |
| 背景颜色和本地图片 | 旧 `BasicWindow` 背景绘制与 ImgAsBackGround | 逐窗口背景设置、图片缓存、RGBA 颜色；布局备份可以嵌入图片。相对图片先按配置目录查找，再尝试程序目录 |
| 时间、当前/下节课程、下节倒计时、多个目标倒计日 | 旧 `SingleTextItem` 的对应业务分支，`Timetable` 查询课程 | `ElementLayout::evaluate/countdown` 和 `ScheduleStore`；时钟 strftime、普通 `{value}` 模板、按天时分秒的 `%d` / `%03d` 等整数占位符；新增教学周和调休规则 |
| 每日 Infos 多条数据及每 5 秒逐条轮换 | 旧 `Timetable::GetInfo`、TodayInfo 分支、Settings_New 信息编辑 | `ScheduleStore::dayInfos` 导入/保存，“每周值日 / 提示”按分号编辑；自由元素“每日信息逐条轮换”每 5 秒切换，也可合并展示 |
| 今日完整课表、首字/全名、竖排文字与当前课高亮 | 旧 `todayalllessons.cpp` 真实绘制并更新自身 rect | `ElementLayout::paint/naturalSize/contentBounds`；元素按内容自动尺寸与窗口 AutoResize 配合，数据刷新时重新适配；高亮和字体度量采用新版实现 |
| 高德今日/明日天气预报及地址自动 Adcode | 旧 `Timetable::GetWeather` 的 `extensions=all`、casts 及 `GetAdcode`，天气条目 APIKey/Address/City | `WeatherClient::text/parseWeather/parseAdcode` 异步请求实况/预报及地理编码；元素编辑器配置 Key、Adcode、城市、地址、预报开关，旧天气数据保留并迁移 |
| 课程编辑、旧 Lessons.json、CSV 追加/转换 | 旧 `Timetable::AddLesson`、`importfromfile.cpp` 选源和目标、Settings_New 编辑入口 | `ScheduleStore::importFile` Replace/Append，主窗口明确选择模式；CSV 引号换行支持；设置中转换为独立 JSON，保留活动课表。课程、Infos、调休追加保留，重复内容去重 |
| 自定义 ConfigFile / LessonInfoFile 的使用意图 | 旧 Settings_New 选择及应用两种路径 | 桌面窗口管理页“更改配置路径”，课程设置“选择课表文件/课表另存为/恢复默认课表”；持久化两类活动路径。不会自动跟随旧指针文件一步迁移所有数据 |
| 可复制的桌面配置备份 | 旧 Windows/UIElements 序列化为 config.json | `DesktopLayoutStore` 原子读写与完整校验；“备份完整布局/恢复完整布局”覆盖窗口、元素、样式、尺寸、启动偏好和背景图片。课表导出仍是独立操作 |
| 系统托盘和 Windows 开机自启 | 旧 `mainwindow.cpp` 托盘及启动项逻辑 | 主窗口托盘菜单、桌面管理页自启开关，HKCU Run 显式引用 EXE 路径 |
| Windows 安装/卸载的发行流程 | 旧 Inno Setup 脚本含开发机绝对路径 | 新 `packaging/windows/TimeTableNext.iss` 与 Windows 工作流提供 Qt5/Qt6 当前用户安装包、快捷方式、可选自启、升级和卸载保留用户数据；未在 Linux 生成或实机验收安装包 |

上述代码路径均在 `modern/`，旧依据路径均在 `TimeTable-Qt/`。旧窗口导入由 `DesktopLayoutStore::fromLegacy` 统一处理 MainWindow/Windows，跳过计数不再双重累加；导入追加窗口，不覆盖课表。

## 已补齐的旧占位项与新增能力

| 项目 | 旧版实际状态 | 当前实现 |
| --- | --- | --- |
| News / DailyWord | `SingleTextItem.cpp` 的两个 case 落入固定 TextFormat，没有内容源 | `ContentClient` 支持可选 HTTPS RSS/Atom/JSON 或本机 UTF-8 文件，新闻每 15 秒轮换、每日一句按日期选取。旧类型的固定 TextFormat 也保留；需要用户配置数据源，没有预置联网源 |
| CurrentLessonProgress / WeatherBlock / Clock 专用块 | `UIElementBase.cpp` 的单项工厂落入不支持异常，不能从枚举推断可用 | 新增 progressBar、weatherBlock、analogClock，元素编辑器可选择并真实绘制；文字进度仍可单独使用 |
| MultiItemAllDisplay | 旧 `CreateMultiItemAllDisplayUIElement` 返回空指针，BasicWindow 未提供真实同时绘制 | `all` 支持纵向、横向、可配置列数网格，同时显示多个条目 |
| 可视化编辑、多选、吸附、分组 | 旧 `EditUIElements::GetCurrentElement` 返回 nullptr，保存、字体、增删、刷新等槽基本为空 | 新编辑器支持拖拽、Ctrl 多选、框选、网格/边缘/中心吸附、六种对齐、组合/解组、撤销/重做，保存/取消使用工作副本 |
| 亚克力 | 旧真实入口 BasicWindow 仅保存 AcrylicEffect；实验类不能证明该入口可用 | `WindowEffects::applyAcrylic` 动态尝试 Windows DWM/Composition/旧 blur 接口；逐窗口开关，不支持时回退。当前只能确认实现路径，视觉效果未实机验证 |
| 课程通知、多课表/学期、冲突检测和搜索 | 未发现旧入口的完整实现，属于历史计划/新增需求 | 默认关闭的课程托盘提醒、提前分钟和跨重启去重；命名课表路径档案；多条件搜索；教学周与时间重叠报告，不阻止保存 |
| 中英界面 | 翻译文件/计划不等于完整运行中的国际化 | `AppTranslator` + 内嵌 English catalog；设置选中文/English，重启生效，用户输入不翻译。支持英语 All/odd/even 周数，不承诺所有 Qt 原生对话框和第三方内容都被翻译 |

## 仍未实现或没有完全还原的事项

| 状态 | 具体缺口及影响 | 当前代码证据 |
| --- | --- | --- |
| **未实现** | 一步读取旧指针配置并迁移课表、布局及相关资源；单个总备份包含课表、布局、学期档案、提醒/语言等偏好和外部内容文件 | `importLegacyWindows` 只传旧窗口给 `fromLegacy`；课表另走 `importSchedule`。布局导出只序列化 OverlayConfig 并内嵌背景，QSettings 档案/偏好和新闻 SourceFile 仅留路径 |
| **未实现** | 单次取消某节课、独立日期事件/考试日程类型、教学楼位置地图 | `ScheduleStore.h` 仅 Course 和整日 DayChange；room 为普通文本。可把考试建成指定教学周课程，但没有独立事件模型或地图 |
| **未实现** | 桌面窗口靠屏幕边缘吸附、全局隐藏/恢复快捷键 | `OverlayCard::mouseMoveEvent` 直接 move 原点+位移；仅元素画布有 SelectionGeometry 吸附。QShortcut 仅在元素编辑对话框注册撤销/分组等操作，未注册系统全局热键 |
| **未实现** | 教务平台专用导入模板、WakeUp 备份适配、账户/手机同步；ICS 导入和显式时区 | `importFile` 分支仅 CSV、新版/旧版 JSON、小爱 courseInfos；`exportIcs` 使用本地浮动 DTSTART/DTEND，无 TZID，且无 importIcs。现有小爱结构兼容不能视为官方手机协议 |
| **部分兼容** | 任意旧布局像素级还原、全部 printf/strftime 扩展、超限元素或窗口参数 | 元素几何/字号、条目数量、字符串长度和窗口范围存在安全上限；旧窗口导入采用无新增边距/圆角及 100% opacity，但 Qt 字体度量、高亮样式、显示器可见范围校正会改变外观。countdown 只支持受限整数占位符 |
| **实现待验收** | Win7/Win10/Win11 多屏、高 DPI、真实滚动性能、透明/亚克力效果、托盘通知、自启、安装/升级/卸载、线上天气和订阅 | 自动化使用 Linux Qt 和离屏窗口/响应样本；Windows 工作流已配置，尚不能据此确认产物或实机行为。需要对应系统和用户授权的真实服务配置验证 |

这些缺口中，一步迁移、像素兼容属于迁移边界；事件、地图、全局热键、教务/WakeUp 等属于旧计划或新增需求，不能全部归为“丢失的成熟旧功能”。

## 数据保护与配置边界

课表和布局使用独立 JSON 文件，QSettings 保存路径、档案、提醒、主题和语言。默认文件为 AppDataLocation 下的 `schedule.json` / `desktop-layout.json`，活动路径可分别更改。

- 路径切换、导入、恢复都先完整校验再提交；写文件使用 QSaveFile。失败或取消保留活动状态和原始文件。
- 启动课表读取失败会设置 `canSaveSchedule_=false`，内存编辑和外观保存不会写回损坏文件。新增必需 courses 数组及成员类型检查，避免合法 schema 下错误类型被当作空课表。可导出/另存新文件，或明确有效替换导入解除保护。
- 布局加载失败或自定义布局丢失会暂停自动保存；可恢复有效备份或切换布局路径。恢复是替换当前布局，旧窗口导入是追加，两者操作不同。
- 原生课表跨开学日期追加被拒绝，避免教学周错位；追加保留原有节次设置，冲突日期以原调休为准。
- API Key 明文存在布局元素中，布局备份包含该 Key。背景图片可嵌入；新闻/每日一句本地文件只保存路径，需要随迁移另行复制。全无数据源配置时不发起网络请求。

## 验证范围

当前 CMake 注册 `schedule_tests`、`element_tests`、`overlay_tests`、`mainwindow_tests`、`content_tests`、`editor_tests`、`layout_tests`、`translation_tests` 共 8 组回归。覆盖教学周/调休、导入追加/失败事务、CSV 换行、提醒计算、损坏文件原始字节保护、可见动画与隐藏停止、轮播边界、布局及内嵌图片恢复、编辑器选择/对齐/撤销，以及响应解析和中英界面。

本轮 Linux Release 统一构建中，Qt5.15 与 Qt6 的 8 组测试各全部通过（16/16）；两套构建的浅色、深色、页面切换及英文启动检查也全部通过（8/8）。Qt5 编辑器额外通过 ASan 地址检查（平台退出泄漏检查关闭）。构建与测试命令见 [README-NEXT](../README-NEXT.md)。Windows Qt5 作业在 Windows Server 2022 编译，Win7 包仍为兼容候选；安装脚本和工作流已实现，不表示 Linux 已实际生成安装包或检验卸载行为。天气/新闻/每日一句当前验证的是离线响应样本和本地文件，未使用真实 Key 或线上源做验收。
