@echo off
REM Windows 打包脚本 - 创建便携版和安装程序

setlocal enabledelayedexpansion

set APP_NAME=SubtitleEditApp
set VERSION=1.0.0
set BUILD_DIR=build
set DEPLOY_DIR=deploy

echo === 开始打包 %APP_NAME% ===

REM 1. 清理之前的构建
echo 清理旧的构建文件...
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%
if exist *.zip del /q *.zip
if exist *.exe del /q *.exe

REM 2. 配置 CMake
echo 配置项目...
cmake -B %BUILD_DIR% -S . -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo 错误: CMake 配置失败
    exit /b 1
)

REM 3. 编译
echo 编译项目...
cmake --build %BUILD_DIR% --config Release
if errorlevel 1 (
    echo 错误: 编译失败
    exit /b 1
)

REM 4. 创建部署目录
echo 准备部署文件...
mkdir %DEPLOY_DIR%
copy %BUILD_DIR%\Release\%APP_NAME%.exe %DEPLOY_DIR%\

REM 5. 使用 windeployqt 部署 Qt 依赖
echo 部署 Qt 框架...
where windeployqt >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 windeployqt，请确保 Qt 已添加到 PATH
    echo 提示: 运行 Qt 安装目录下的 qtenv2.bat
    exit /b 1
)

windeployqt %DEPLOY_DIR%\%APP_NAME%.exe --release --no-translations

REM 6. 添加 README
echo 创建 README...
(
echo SRT 字幕编辑器 v%VERSION%
echo.
echo 使用说明：
echo 直接运行 %APP_NAME%.exe 即可
echo.
echo 功能特性：
echo - 打开和编辑 SRT 字幕文件
echo - 时间平移功能
echo - 点同步功能（通过参考字幕）
echo - 智能时间匹配高亮
echo.
echo 快捷键：
echo Ctrl+O - 打开文件
echo Ctrl+S - 保存
echo Ctrl+T - 时间平移
echo Ctrl+P - 点同步
echo.
echo 更多信息请访问项目主页。
) > %DEPLOY_DIR%\README.txt

REM 7. 创建便携版 ZIP
echo 创建便携版 ZIP...
powershell -Command "Compress-Archive -Path %DEPLOY_DIR%\* -DestinationPath %APP_NAME%-%VERSION%-Windows-Portable.zip -Force"

echo.
echo === 打包完成! ===
echo 便携版: %APP_NAME%-%VERSION%-Windows-Portable.zip
for %%F in (%APP_NAME%-%VERSION%-Windows-Portable.zip) do echo 大小: %%~zF 字节
echo.
echo 如需创建安装程序，请安装 NSIS 并运行相应的脚本
echo.

endlocal
