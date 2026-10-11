# CI 与 Release 发布流程

## 工作流分工

- `.github/workflows/modern-build.yml`：在 PR、master 推送和手动触发时构建与测试；也作为 `workflow_call` 被 Release 调用。它是唯一维护 Qt 安装、编译、测试和打包步骤的地方。
- `.github/workflows/release.yml`：只处理版本校验、复用构建、核对附件、生成 SHA256、发布或补全 Release；不再复制构建步骤。

PR 会获得 Actions artifacts 供下载测试，但**不会自动发布**。主分支改动会运行 CI。通过正式版本 Tag（例如 `v4.2.1`）推送才会自动触发发布。工作流也支持手动指定**已经存在**的 Tag。

## 发版步骤

1. 将经 CI 验证的 PR 合并至 `master`，将 `CMakeLists.txt` 里的 `project(TimeTableNext VERSION x.y.z ...)` 更新为目标版本并提交。
2. 确保该提交测试通过，再创建与 CMake 版本严格一致的轻量或附注 Tag，例如：

   ```bash
   git checkout master
   git pull --ff-only
   git tag v4.2.1
   git push origin v4.2.1
   ```

3. Actions → **Publish TimeTable Next** 会验证 Tag，调用同一套构建与打包任务，全部成功后才发布带安装包的 Release。失败时不会生成新的正式 Release。
4. 如果 Tag 已经存在但没有触发发布，打开 Actions → **Publish TimeTable Next** → **Run workflow**，选择包含最新版工作流的 `master` 并输入 `tag`（如 `v4.2.0`）。**不能输入尚未创建的 Tag**。

## 产物

每次正式发布预期有 **12 个包 + SHA256SUMS.txt**：

| 系统 | 内容 | 数量 |
| --- | --- | --- |
| Windows x64 Qt6 | 便携 ZIP、Inno Setup 安装程序 | 2 |
| Windows x64 Qt5 | 便携 ZIP、Inno Setup 安装程序（Win7 兼容候选，未实机认证） | 2 |
| Ubuntu 24.04 | Qt5 / Qt6 各有 DEB 和 tar.gz | 4 |
| Debian 13 | Qt5 / Qt6 各有 DEB 和 tar.gz | 4 |

Linux 包按**目标发行版分别编译、测试和打包**。请不要将 Ubuntu 的 DEB 装到 Debian 上；tar.gz 也不是完全静态、无需依赖的单文件程序。Qt5 与 Qt6 DEB 的系统包名相同，二选一安装。macOS 尚无受验证的发行包。

## 防止错误发布

- 仅支持形如 `vX.Y.Z` 的正式版本 Tag，且必须与**Tag 指向的源码**中的 CMake 版本相同。
- 任一平台编译、CTest、冒烟测试或打包失败，都不会进入发布步骤。
- 发布前检查 12 个附件是否齐全、非空，并生成校验和。
- 已有附件的 Release 不会被自动覆盖；空附件的 Release（包括已经手动创建的 v4.2.0）可以通过手动重跑补齐。
- 新 Release 先创建草稿并上传附件，完成后才公开；已经公开但没有附件的 Release 会直接补上传。
- 同一 Tag 的发布按顺序执行，避免同时上传。
- CI 绿灯不代表 Win7 实机兼容、安装器升级/卸载、托盘及多显示器功能都已人工验收。

详见 [平台安装与本地打包](../packaging/README.md)。
