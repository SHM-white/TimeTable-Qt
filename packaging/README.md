# Windows 安装包与便携包

`.github/workflows/modern-build.yml` 在原有 Qt 5/Qt 6 便携 ZIP 之外，分别生成 Inno Setup 安装包 artifact。安装包不会自动发布到 Releases。Qt 5 和 Qt 6 安装包使用同一应用标识，安装其中一个会升级或替换当前安装；不会覆盖旧版 v3 的安装标识。

| 构建 | ZIP artifact（保留原名） | 安装包 artifact | 系统范围 |
| --- | --- | --- | --- |
| Qt 6.8.3 / x64 | `TimeTableNext-Windows-x64` | `TimeTableNext-Windows-x64-Installer` | Windows 10 1809 及以上、Windows 11 |
| Qt 5.15.2 / x64 | `TimeTableNext-Qt5-Win7-Candidate` | `TimeTableNext-Qt5-Windows-x64-Installer` | Windows 7 SP1 及以上的兼容候选；尚未在 Win7 实机验证 |

Qt 5 安装包使用 Inno Setup **6.2.2** 编译，避免较新安装器引擎改变 Windows 最低版本。CI 优先使用已安装的相同编译器版本，否则通过 Chocolatey 安装该固定版本。现代 Qt 6 构建可使用已安装的 Inno Setup 6 或同一个固定版本。Windows 7 仍需验证 VC Runtime、HTTPS、平台插件、高 DPI 和托盘行为；编译成功不能证明它在 Win7 上可运行。

安装默认只针对当前用户，目录为 `%LOCALAPPDATA%\Programs\TimeTableNext`，不要求管理员权限。安装向导可选择桌面快捷方式和登录 Windows 后自动启动；默认均不勾选。开机启动与程序内的设置使用同一个 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\TimeTableNext` 值。卸载清除仍指向该安装目录的启动项，并保留课表文件、布局、天气配置等用户数据。

便携包解压后运行 `TimeTableNext.exe`；“便携”描述程序分发形式，程序仍使用自己的用户数据目录及 QSettings。配置和课表位置以程序设置为准，数据不随卸载而删除。

## 本地生成安装包

先编译并部署全部 Qt 运行库，再调用 Inno Setup 6.2.2 的 `ISCC.exe`。使用已经部署的目录作为 `SourceDir`；脚本会检查主程序、对应版本的 QtCore 和 `platforms\qwindows.dll`，缺失时直接失败。版本从项目构建版本传入，不在脚本中重复硬编码。

在仓库根目录运行 PowerShell，例如：

```powershell
New-Item -ItemType Directory -Force dist/TimeTableNext | Out-Null
Copy-Item build/bin/TimeTableNext.exe dist/TimeTableNext/
windeployqt --release --no-translations --dir dist/TimeTableNext dist/TimeTableNext/TimeTableNext.exe
$versionMatch = [regex]::Match((Get-Content CMakeLists.txt -Raw), '(?m)^\s*project\(TimeTableNext\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)')
if (-not $versionMatch.Success) { throw 'Cannot read the CMake project version' }
$iscc = Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6/ISCC.exe'
& $iscc "/DAppVersion=$($versionMatch.Groups[1].Value)" `
  "/DSourceDir=$((Resolve-Path dist/TimeTableNext).Path)" `
  "/DOutputDir=$PWD/dist/installers" `
  packaging/windows/TimeTableNext.iss
if ($LASTEXITCODE -ne 0) { throw 'Inno Setup compilation failed' }
```

Qt 5 使用对应 Qt 5 的 `windeployqt`，增加 `/DQt5Legacy` 参数，并使用 Inno Setup 6.2.2 编译。没有该参数时脚本构建 Qt 6 安装包。输出为 `TimeTableNext-<版本>-Qt<5或6>-Windows-x64-Setup.exe`。

安装包尚需由 Windows CI 实际编译并在目标 Windows 系统验收；Linux 静态检查不能替代安装、升级和卸载测试。

## Linux 包（Ubuntu 24.04 AMD64）

`modern-build.yml` 的 Linux Qt6 和 Qt5 任务先编译、运行 CTest 和无界面启动检查，全部通过后各自产生两个包，并上传为 Actions artifacts：

| Qt 构建 | artifact | 内含文件 |
| --- | --- | --- |
| Qt6 | `TimeTableNext-Linux-Qt6` | `TimeTableNext-v4.2.0-Linux-Qt6-amd64.deb`、对应 `.tar.gz` |
| Qt5 | `TimeTableNext-Linux-Qt5` | `TimeTableNext-v4.2.0-Linux-Qt5-amd64.deb`、对应 `.tar.gz` |

版本号实际由 `CMakeLists.txt` 读取，上表仅为当前版本示例。两种 DEB 都使用 Debian 包名 `timetable-next`，因此应二选一安装（安装另一个会替换原版本）。它们使用 Ubuntu 24.04 的系统 Qt 依赖，不包含 Qt 运行库，也不保证在 Debian 12、Ubuntu 22.04 或其他发行版上直接可安装。

下载对应 DEB 后在其所在目录执行：

```bash
sudo apt install ./TimeTableNext-v4.2.0-Linux-Qt6-amd64.deb
TimeTableNext
```

也可以从应用启动菜单打开。打包脚本 `packaging/linux/package.sh` 在仓库根目录执行，例如 `bash packaging/linux/package.sh build Qt6`；还生成可解压运行的 Linux tar.gz（仍需系统自行安装 Qt 运行时）。Linux 安装包与 Windows 安装包一样只上传 CI artifacts，**不会由开发分支自动发布 Release**；正式发布工作流 `release.yml` 目前仍是历史 v4.1.0 发布配置。
