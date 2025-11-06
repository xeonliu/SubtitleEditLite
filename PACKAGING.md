# 打包和发布指南

## 本地打包

### macOS

1. **准备环境**
   ```bash
   # 确保已安装 Qt 和 CMake
   brew install qt cmake
   ```

2. **运行打包脚本**
   ```bash
   ./package_macos.sh
   ```

3. **输出文件**
   - `SubtitleEditApp-1.0.0-macOS.dmg` - 安装包

4. **手动打包（可选）**
   ```bash
   # 构建 Release 版本
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   
   # 部署 Qt 依赖并创建 DMG
   macdeployqt build/SubtitleEditApp.app -dmg
   ```

### Windows

1. **准备环境**
   - 安装 Qt (推荐 6.7+)
   - 安装 CMake 和 Visual Studio
   - 将 Qt 的 bin 目录添加到 PATH 或运行 `qtenv2.bat`

2. **运行打包脚本**
   ```cmd
   package_windows.bat
   ```

3. **输出文件**
   - `SubtitleEditApp-1.0.0-Windows-Portable.zip` - 便携版

4. **手动打包（可选）**
   ```cmd
   # 构建 Release 版本
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   
   # 创建部署目录
   mkdir deploy
   copy build\Release\SubtitleEditApp.exe deploy\
   
   # 部署 Qt 依赖
   windeployqt deploy\SubtitleEditApp.exe --release
   ```

### Linux

```bash
# 构建 Release 版本
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 安装到临时目录
cmake --install build --prefix AppDir/usr

# 使用 linuxdeploy 创建 AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
```

---

## GitHub Actions 自动发布

### 设置步骤

1. **推送代码到 GitHub**
   ```bash
   git init
   git add .
   git commit -m "Initial commit"
   git remote add origin https://github.com/yourusername/SubtitleEditApp.git
   git push -u origin main
   ```

2. **创建版本标签**
   ```bash
   # 创建并推送标签
   git tag v1.0.0
   git push origin v1.0.0
   ```

3. **自动构建**
   - GitHub Actions 会自动检测标签推送
   - 同时构建 macOS、Windows 和 Linux 版本
   - 构建完成后自动创建 GitHub Release
   - 所有安装包会自动上传到 Release 页面

### 手动触发构建

如果不想创建标签，也可以手动触发：

1. 打开 GitHub 仓库页面
2. 点击 "Actions" 标签
3. 选择 "Build and Release" 工作流
4. 点击 "Run workflow" 按钮
5. 选择分支并运行

### 工作流功能

GitHub Actions 会自动执行以下操作：

#### macOS 构建
- 安装 Qt 6.7
- 编译 Release 版本
- 使用 `macdeployqt` 打包
- 生成 `.dmg` 安装包
- 上传到 Release

#### Windows 构建
- 安装 Qt 6.7 和 MSVC
- 编译 Release 版本
- 使用 `windeployqt` 打包
- 生成 NSIS 安装程序 (`.exe`)
- 生成便携版 ZIP
- 上传到 Release

#### Linux 构建
- 安装 Qt 6.7
- 编译 Release 版本
- 创建 AppImage (可在任何 Linux 发行版运行)
- 上传到 Release

### 输出文件

每次发布会生成以下文件：

```
SubtitleEditApp-macOS.dmg              - macOS 安装包
SubtitleEditApp-Windows-Setup.exe     - Windows 安装程序
SubtitleEditApp-Windows-Portable.zip  - Windows 便携版
SubtitleEditApp-Linux.AppImage        - Linux AppImage
```

---

## 版本发布流程

### 1. 更新版本号

在以下文件中更新版本号：

- `CMakeLists.txt`: `project(SubtitleEditApp VERSION 1.0.0)`
- `package_macos.sh`: `VERSION="1.0.0"`
- `package_windows.bat`: `set VERSION=1.0.0`

### 2. 更新 CHANGELOG

创建 `CHANGELOG.md` 记录更新内容：

```markdown
## v1.0.0 - 2025-11-06

### 新增功能
- SRT 字幕文件的打开、编辑和保存
- 时间平移功能（整体调整字幕时间）
- 点同步功能（通过参考字幕精确同步）
- 智能时间匹配高亮（基于概率分布）
- 实时预览和重置功能

### 改进
- 分段线性变换算法，支持多点同步
- 双表格对照视图
- 可重复调整和预览

### 技术细节
- 基于 Qt 6 开发
- 支持 macOS、Windows 和 Linux
- UTF-8 编码支持
```

### 3. 提交并打标签

```bash
git add .
git commit -m "Release v1.0.0"
git tag -a v1.0.0 -m "Version 1.0.0 - Initial Release"
git push origin main
git push origin v1.0.0
```

### 4. 等待构建完成

- 访问 GitHub Actions 页面查看构建进度
- 构建通常需要 10-20 分钟
- 完成后会在 Releases 页面看到新版本

### 5. 编辑 Release 说明

1. 访问 GitHub Releases 页面
2. 点击自动创建的 Release
3. 点击 "Edit" 编辑
4. 添加详细的发布说明（可以粘贴 CHANGELOG 内容）
5. 保存

---

## 故障排除

### macOS 构建失败

**问题**: `macdeployqt not found`

**解决**:
```bash
# 确保 Qt bin 目录在 PATH 中
export PATH="/usr/local/opt/qt/bin:$PATH"
```

### Windows 构建失败

**问题**: `windeployqt not found`

**解决**:
```cmd
# 运行 Qt 环境设置脚本
C:\Qt\6.7.0\msvc2019_64\bin\qtenv2.bat

# 或者手动添加到 PATH
set PATH=C:\Qt\6.7.0\msvc2019_64\bin;%PATH%
```

**问题**: `NSIS not found`

**解决**:
```cmd
# 安装 NSIS
choco install nsis

# 或从官网下载: https://nsis.sourceforge.io/
```

### GitHub Actions 失败

**问题**: Actions 权限不足

**解决**:
1. 进入仓库 Settings
2. 点击 Actions > General
3. 滚动到 "Workflow permissions"
4. 选择 "Read and write permissions"
5. 保存

**问题**: Qt 安装超时

**解决**:
- GitHub Actions 中 Qt 缓存默认启用
- 首次构建可能较慢，后续会快很多
- 如果仍然超时，可以减少并行构建任务

---

## 发布检查清单

发布前确保：

- [ ] 所有功能测试通过
- [ ] 更新了版本号
- [ ] 更新了 CHANGELOG
- [ ] 提交了所有更改
- [ ] 创建并推送了版本标签
- [ ] 本地打包脚本能正常运行
- [ ] GitHub Actions 构建成功
- [ ] 所有平台的安装包都能正常工作
- [ ] Release 说明完整

---

## 高级选项

### 代码签名

#### macOS

```bash
# 签名应用
codesign --deep --force --sign "Developer ID Application: Your Name" \
    build/SubtitleEditApp.app

# 公证（需要 Apple Developer 账号）
xcrun notarytool submit SubtitleEditApp-macOS.dmg \
    --apple-id "your@email.com" \
    --team-id "TEAMID" \
    --password "app-specific-password"
```

#### Windows

```cmd
# 使用 signtool 签名（需要代码签名证书）
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com deploy\SubtitleEditApp.exe
```

### 自定义图标

- macOS: 将 `icon.icns` 添加到 `Info.plist`
- Windows: 在 `CMakeLists.txt` 中添加 `WIN32_EXECUTABLE` 和图标资源
- Linux: 在 AppImage 构建时指定图标

---

## 持续集成最佳实践

1. **使用缓存**: GitHub Actions 已配置 Qt 缓存，加速构建
2. **并行构建**: 三个平台同时构建，节省时间
3. **自动测试**: 可以添加单元测试步骤
4. **发布草稿**: 可以先发布为草稿，测试后再公开
5. **版本管理**: 使用语义化版本号 (MAJOR.MINOR.PATCH)

---

## 参考资料

- [Qt Documentation - Deploying Applications](https://doc.qt.io/qt-6/deployment.html)
- [GitHub Actions - Building and testing](https://docs.github.com/en/actions)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [linuxdeploy Documentation](https://github.com/linuxdeploy/linuxdeploy)
