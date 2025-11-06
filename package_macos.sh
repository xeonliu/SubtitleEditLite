#!/bin/bash

# macOS 打包脚本 - 创建 DMG 安装包

set -e

APP_NAME="SubtitleEditApp"
VERSION="1.0.0"
BUILD_DIR="build"
DMG_DIR="dmg_temp"
DMG_NAME="${APP_NAME}-${VERSION}-macOS.dmg"

echo "=== 开始打包 ${APP_NAME} ==="

# 0. 检测 Qt 路径
echo "检测 Qt 安装..."
if command -v qmake &> /dev/null; then
    QT_BIN_DIR=$(qmake -query QT_INSTALL_BINS)
    QT_PREFIX=$(qmake -query QT_INSTALL_PREFIX)
    echo "Qt 路径: $QT_PREFIX"
    echo "Qt bin: $QT_BIN_DIR"
    export PATH="$QT_BIN_DIR:$PATH"
else
    echo "警告: 未找到 qmake，可能无法正确部署 Qt"
fi

# 1. 清理之前的构建
echo "清理旧的构建文件..."
rm -rf ${BUILD_DIR}
rm -rf ${DMG_DIR}
rm -f *.dmg

# 2. 使用 CMake 构建 Release 版本
echo "配置项目..."
cmake -B ${BUILD_DIR} -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX"

echo "编译项目..."
cmake --build ${BUILD_DIR} --config Release

# 3. 检查 App 是否存在
if [ ! -d "${BUILD_DIR}/${APP_NAME}.app" ]; then
    echo "错误: 未找到 ${APP_NAME}.app"
    exit 1
fi

echo "App 构建成功: ${BUILD_DIR}/${APP_NAME}.app"

# 4. 查找并使用 macdeployqt
echo "查找 macdeployqt..."
MACDEPLOYQT=""

if [ -n "$QT_BIN_DIR" ] && [ -f "$QT_BIN_DIR/macdeployqt" ]; then
    MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"
    echo "找到 macdeployqt: $MACDEPLOYQT"
elif command -v macdeployqt &> /dev/null; then
    MACDEPLOYQT=$(which macdeployqt)
    echo "通过 which 找到 macdeployqt: $MACDEPLOYQT"
else
    echo "错误: 未找到 macdeployqt，请确保 Qt 已正确安装"
    exit 1
fi

# 5. 使用 macdeployqt 打包 Qt 框架
echo "使用 macdeployqt 部署 Qt 框架..."
echo "Qt 库路径: $QT_PREFIX/lib"

# 设置 DYLD_LIBRARY_PATH 以帮助 macdeployqt 找到 Qt 框架
export DYLD_LIBRARY_PATH="$QT_PREFIX/lib:$DYLD_LIBRARY_PATH"

# 运行 macdeployqt，明确指定 Qt 路径和所需模块
# 注意：添加 -always-overwrite 确保正确部署所有依赖
$MACDEPLOYQT ${BUILD_DIR}/${APP_NAME}.app \
    -verbose=2 \
    -libpath="$QT_PREFIX/lib" \
    -always-overwrite

if [ $? -ne 0 ]; then
    echo "警告: macdeployqt 报告了一些错误，但继续处理..."
fi

# 5.1 手动复制 QtDBus 框架（macdeployqt 有时会遗漏）
echo "检查并复制 QtDBus 框架..."
QTDBUS_SOURCE="$QT_PREFIX/lib/QtDBus.framework"
QTDBUS_TARGET="${BUILD_DIR}/${APP_NAME}.app/Contents/Frameworks/QtDBus.framework"

# 删除可能存在的符号链接或不完整的目录
if [ -L "$QTDBUS_TARGET" ] || [ -d "$QTDBUS_TARGET" ]; then
    echo "删除现有的 QtDBus..."
    rm -rf "$QTDBUS_TARGET"
fi

if [ -d "$QTDBUS_SOURCE" ]; then
    echo "复制 QtDBus 框架..."
    
    # 创建正确的框架结构
    mkdir -p "$QTDBUS_TARGET/Versions/A"
    
    # 复制二进制文件
    cp "$QTDBUS_SOURCE/Versions/A/QtDBus" "$QTDBUS_TARGET/Versions/A/QtDBus"
    
    # 复制资源文件（如果存在）
    if [ -d "$QTDBUS_SOURCE/Versions/A/Resources" ]; then
        cp -R "$QTDBUS_SOURCE/Versions/A/Resources" "$QTDBUS_TARGET/Versions/A/"
    fi
    
    # 创建符号链接
    cd "$QTDBUS_TARGET"
    ln -s A Versions/Current
    ln -s Versions/Current/QtDBus QtDBus
    if [ -d "Versions/A/Resources" ]; then
        ln -s Versions/Current/Resources Resources
    fi
    cd - > /dev/null
    
    # 修复 QtDBus 的 rpath
    echo "修复 QtDBus 的依赖路径..."
    install_name_tool -id "@rpath/QtDBus.framework/Versions/A/QtDBus" \
        "$QTDBUS_TARGET/Versions/A/QtDBus" 2>/dev/null || true
        
    # 修复 QtDBus 对 QtCore 的引用
    install_name_tool -change "$QT_PREFIX/lib/QtCore.framework/Versions/A/QtCore" \
        "@rpath/QtCore.framework/Versions/A/QtCore" \
        "$QTDBUS_TARGET/Versions/A/QtDBus" 2>/dev/null || true
        
    # 修复 QtDBus 对 QtDBus 自身的引用（可能存在）
    install_name_tool -change "$QT_PREFIX/lib/QtDBus.framework/Versions/A/QtDBus" \
        "@rpath/QtDBus.framework/Versions/A/QtDBus" \
        "$QTDBUS_TARGET/Versions/A/QtDBus" 2>/dev/null || true
    
    # 复制 libdbus-1 动态库（QtDBus 依赖）
    echo "复制 libdbus-1 库..."
    LIBDBUS_SOURCE=$(otool -L "$QTDBUS_TARGET/Versions/A/QtDBus" | grep libdbus | awk '{print $1}')
    if [ -n "$LIBDBUS_SOURCE" ] && [ -f "$LIBDBUS_SOURCE" ]; then
        cp "$LIBDBUS_SOURCE" "${BUILD_DIR}/${APP_NAME}.app/Contents/Frameworks/"
        LIBDBUS_NAME=$(basename "$LIBDBUS_SOURCE")
        
        # 修复 QtDBus 对 libdbus 的引用
        install_name_tool -change "$LIBDBUS_SOURCE" \
            "@rpath/$LIBDBUS_NAME" \
            "$QTDBUS_TARGET/Versions/A/QtDBus" 2>/dev/null || true
        
        echo "✓ libdbus-1 已复制并修复"
    fi
        
    echo "✓ QtDBus 框架已复制并修复"
else
    echo "警告: 未找到 QtDBus 源框架，继续..."
fi

# 6. 重新签名整个 bundle（macdeployqt 会修改文件，导致签名失效）
echo "重新对 App Bundle 进行代码签名..."

# 先签名所有框架和库
echo "签名框架和库..."
find ${BUILD_DIR}/${APP_NAME}.app/Contents/Frameworks -name "*.dylib" -o -name "*.framework" | while read lib; do
    codesign --force --sign - "$lib" 2>/dev/null || true
done

# 签名所有插件
echo "签名插件..."
find ${BUILD_DIR}/${APP_NAME}.app/Contents/PlugIns -name "*.dylib" | while read plugin; do
    codesign --force --sign - "$plugin" 2>/dev/null || true
done

# 最后签名整个 bundle
echo "签名应用 bundle..."
codesign --force --deep --sign - ${BUILD_DIR}/${APP_NAME}.app

if [ $? -ne 0 ]; then
    echo "错误: 代码签名失败"
    exit 1
fi

echo "代码签名完成"

# 7. 验证签名
echo "验证代码签名..."
codesign --verify --deep --verbose ${BUILD_DIR}/${APP_NAME}.app
if [ $? -eq 0 ]; then
    echo "✓ 签名验证成功"
else
    echo "✗ 签名验证失败，但继续打包..."
fi

# 8. 创建 DMG 临时目录
echo "准备 DMG 目录..."
mkdir -p ${DMG_DIR}
cp -R ${BUILD_DIR}/${APP_NAME}.app ${DMG_DIR}/

# 创建 Applications 快捷方式
ln -s /Applications ${DMG_DIR}/Applications

# 添加 README
cat > ${DMG_DIR}/README.txt << EOF
SRT 字幕编辑器 v${VERSION}

安装说明：
将 ${APP_NAME}.app 拖到 Applications 文件夹即可

功能特性：
- 打开和编辑 SRT 字幕文件
- 时间平移功能
- 点同步功能（通过参考字幕）
- 智能时间匹配高亮

使用方法：
1. 文件 > 打开 (Cmd+O) 加载 SRT 文件
2. 编辑 > 时间平移 (Cmd+T) 整体调整时间
3. 编辑 > 点同步 (Cmd+P) 通过参考字幕精确同步

更多信息请访问项目主页。
EOF

# 9. 创建 DMG
echo "创建 DMG 镜像..."
hdiutil create -volname "${APP_NAME}" \
    -srcfolder ${DMG_DIR} \
    -ov -format UDZO \
    ${DMG_NAME}

# 10. 清理临时文件
echo "清理临时文件..."
rm -rf ${DMG_DIR}

echo "=== 打包完成! ==="
echo "DMG 文件: ${DMG_NAME}"
echo "大小: $(du -h ${DMG_NAME} | cut -f1)"

# 11. 验证 DMG
if command -v hdiutil &> /dev/null; then
    echo "验证 DMG..."
    hdiutil verify ${DMG_NAME}
    echo "验证成功!"
fi

echo ""
echo "现在可以分发 ${DMG_NAME} 文件了！"
echo ""
echo "测试提示："
echo "1. 挂载 DMG: open ${DMG_NAME}"
echo "2. 拖动应用到 Applications"
echo "3. 运行应用测试"
