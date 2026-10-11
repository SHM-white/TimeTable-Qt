# Windows 安装包与便携包

`.github/workflows/modern-build.yml` 在原有 Qt 5/Qt 6 便携 ZIP 之外，分别生成 Inno Setup 安装包 artifact。正常 PR 和主分支 CI 仅上传 Actions artifacts；推送正式版本 Tag 时，Release 工作流会复用构建结果，将 ZIP 与安装包一同发布。Qt 5 和 Qt 6 安装包使用同一应用标识，安装其中一个会升级或替换当前安装；不会覆盖旧版 v3 的安装标识。

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

## Linux DEB 与 tar.gz（按发行版分别构建）

CI 在目标发行版上分别编译、测试和打包。Ubuntu 24.04 使用 GitHub Ubuntu runner，Debian 13 使用 `debian:trixie` 容器。两套 Qt 5 / Qt 6 均要通过 CTest、深浅色和英文界面冒烟测试，打包后再模拟 APT 安装。

| 目标 | Actions artifact | DEB 文件名（4.2.0 示例） |
| --- | --- | --- |
| Debian 13 Qt6 | `TimeTableNext-Debian13-Qt6` | `TimeTableNext-v4.2.0-Linux-Debian13-Qt6-amd64.deb` |
| Debian 13 Qt5 | `TimeTableNext-Debian13-Qt5` | `TimeTableNext-v4.2.0-Linux-Debian13-Qt5-amd64.deb` |
| Ubuntu 24.04 Qt6 | `TimeTableNext-Ubuntu24.04-Qt6` | `TimeTableNext-v4.2.0-Linux-Ubuntu24.04-Qt6-amd64.deb` |
| Ubuntu 24.04 Qt5 | `TimeTableNext-Ubuntu24.04-Qt5` | `TimeTableNext-v4.2.0-Linux-Ubuntu24.04-Qt5-amd64.deb` |

每个 artifact 还包括相同平台和 Qt 版本的 `.tar.gz`。版本号从 `CMakeLists.txt` 提取；所有 DEB 包名均为 `timetable-next`，Qt 5 和 Qt 6 应二选一安装。

**请勿在 Debian 上安装 Ubuntu 版 DEB。** Debian 13 Qt6 依赖 `libqt6widgets6`、`libqt6network6`，Ubuntu 24.04 则使用 `libqt6widgets6t64`、`libqt6network6t64`。二者还分别使用不同发行版的 Qt 和 glibc 编译环境。

Debian 13 安装示例：

```bash
cat /etc/os-release
sudo apt install ./TimeTableNext-v4.2.0-Linux-Debian13-Qt6-amd64.deb
TimeTableNext
```

本地使用 `bash packaging/linux/package.sh build Qt6` 打包；只支持 Debian 13 和 Ubuntu 24.04 AMD64，且需要在目标发行版已经完成的 CMake 构建目录。CI 检查真实 APT 包名并执行 `apt-get install --simulate`。Debian 12、Ubuntu 22.04 不在本次构建范围。PR / 主分支构建产物保存在 Actions Artifacts；正式版本 Tag 的 Release 工作流复用相同构建过程，发布所有 12 个安装包及 `SHA256SUMS.txt`。发版操作与安全检查见 [CI 和 Release 流程](../docs/CI_RELEASE.md)。
