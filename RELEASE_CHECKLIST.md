# Release Checklist v1.0.0

## 准备阶段

- [ ] 所有计划功能已完成
- [ ] 所有已知 bug 已修复
- [ ] 代码已清理和优化
- [ ] 注释和文档已更新

## 版本管理

- [ ] 更新 `CMakeLists.txt` 中的版本号
- [ ] 更新 `package_macos.sh` 中的版本号  
- [ ] 更新 `package_windows.bat` 中的版本号
- [ ] 创建或更新 `CHANGELOG.md`

## 本地测试

### macOS
- [ ] 编译通过 (`cmake --build build`)
- [ ] 应用可以正常启动
- [ ] 所有功能正常工作
- [ ] 打开/保存文件正常
- [ ] 时间平移功能正常
- [ ] 点同步功能正常
- [ ] 打包脚本运行成功 (`./package_macos.sh`)
- [ ] DMG 可以安装和运行

### Windows (如有条件)
- [ ] 编译通过
- [ ] 应用可以正常启动
- [ ] 所有功能正常工作
- [ ] 打包脚本运行成功
- [ ] ZIP 解压后可以运行

## 文档检查

- [ ] README.md 准确描述当前功能
- [ ] DEMO.md 截图和说明是最新的
- [ ] POINT_SYNC_GUIDE.md 说明准确
- [ ] PACKAGING.md 包含完整的打包说明

## Git 提交

- [ ] 所有更改已提交
  ```bash
  git status  # 确保工作区干净
  ```
- [ ] 提交信息清晰
  ```bash
  git commit -m "Release v1.0.0"
  ```
- [ ] 推送到远程仓库
  ```bash
  git push origin main
  ```

## 创建标签

- [ ] 创建版本标签
  ```bash
  git tag -a v1.0.0 -m "Version 1.0.0 - Initial Release"
  ```
- [ ] 推送标签
  ```bash
  git push origin v1.0.0
  ```

## GitHub Actions

- [ ] 访问 Actions 页面
  https://github.com/yourusername/SubtitleEditApp/actions
  
- [ ] 确认工作流已触发

- [ ] 等待构建完成（通常 10-20 分钟）
  - [ ] macOS 构建成功
  - [ ] Windows 构建成功  
  - [ ] Linux 构建成功

- [ ] 检查构建产物
  - [ ] SubtitleEditApp-macOS.dmg
  - [ ] SubtitleEditApp-Windows-Setup.exe
  - [ ] SubtitleEditApp-Windows-Portable.zip
  - [ ] SubtitleEditApp-Linux.AppImage

## Release 编辑

- [ ] 访问 Releases 页面
- [ ] 编辑自动创建的 Release
- [ ] 添加发布说明（包含 CHANGELOG 内容）
- [ ] 确认所有安装包都已上传
- [ ] 设置为最新版本
- [ ] 发布 Release

## 发布后测试

### macOS
- [ ] 从 Release 下载 DMG
- [ ] 安装到 Applications
- [ ] 启动应用
- [ ] 测试基本功能

### Windows
- [ ] 从 Release 下载安装程序
- [ ] 运行安装程序
- [ ] 启动应用
- [ ] 测试基本功能
- [ ] 下载便携版测试

### Linux
- [ ] 从 Release 下载 AppImage
- [ ] 添加执行权限 (`chmod +x`)
- [ ] 启动应用
- [ ] 测试基本功能

## 宣传和通知

- [ ] 在项目主页添加下载链接
- [ ] 更新社交媒体（如适用）
- [ ] 通知用户和贡献者
- [ ] 在相关论坛或社区发布

## 后续工作

- [ ] 监控 GitHub Issues 中的反馈
- [ ] 记录用户报告的问题
- [ ] 规划下一个版本的功能
- [ ] 创建 Milestone 跟踪进度

---

## 快速命令参考

```bash
# 完整发布流程
git add .
git commit -m "Release v1.0.0"
git push origin main
git tag -a v1.0.0 -m "Version 1.0.0"
git push origin v1.0.0

# 本地打包测试（macOS）
./package_macos.sh

# 查看标签
git tag -l

# 删除本地标签（如果需要重新创建）
git tag -d v1.0.0

# 删除远程标签
git push origin --delete v1.0.0
```

---

## 紧急回滚

如果发现严重问题需要回滚：

1. 删除 Release
   - 在 GitHub 上删除 Release
   
2. 删除标签
   ```bash
   git tag -d v1.0.0
   git push origin --delete v1.0.0
   ```

3. 修复问题后重新发布

---

日期: _____________
发布人: _____________
版本: v1.0.0
状态: ⬜ 准备中 / ⬜ 进行中 / ⬜ 已完成
