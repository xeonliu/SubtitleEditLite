#!/bin/bash

# 测试本地打包流程

echo "=== 测试 macOS 打包 ==="
echo ""

# 检查必要工具
echo "1. 检查必要工具..."

if ! command -v cmake &> /dev/null; then
    echo "❌ 未找到 cmake"
    exit 1
else
    echo "✅ cmake: $(cmake --version | head -n1)"
fi

if ! command -v qmake &> /dev/null && ! find ~/Qt* -name qmake 2>/dev/null | head -n1; then
    echo "⚠️  警告: 未找到 qmake，可能未安装 Qt"
else
    echo "✅ Qt: 已安装"
fi

if ! command -v hdiutil &> /dev/null; then
    echo "❌ 未找到 hdiutil (macOS only)"
else
    echo "✅ hdiutil: 可用"
fi

echo ""
echo "2. 测试编译..."

# 清理并构建
rm -rf build_test
cmake -B build_test -S . -DCMAKE_BUILD_TYPE=Release

if [ $? -eq 0 ]; then
    echo "✅ CMake 配置成功"
else
    echo "❌ CMake 配置失败"
    exit 1
fi

cmake --build build_test --config Release

if [ $? -eq 0 ]; then
    echo "✅ 编译成功"
else
    echo "❌ 编译失败"
    exit 1
fi

# 检查生成的应用
if [ -d "build_test/SubtitleEditApp.app" ]; then
    echo "✅ 应用包生成成功"
    echo "   位置: build_test/SubtitleEditApp.app"
else
    echo "❌ 应用包未生成"
    exit 1
fi

echo ""
echo "3. 检查打包脚本..."

if [ -f "package_macos.sh" ]; then
    echo "✅ package_macos.sh 存在"
    if [ -x "package_macos.sh" ]; then
        echo "✅ 打包脚本可执行"
    else
        echo "⚠️  打包脚本不可执行，尝试修复..."
        chmod +x package_macos.sh
    fi
else
    echo "❌ 未找到 package_macos.sh"
fi

echo ""
echo "=== 测试完成 ==="
echo ""
echo "下一步:"
echo "1. 运行 ./package_macos.sh 生成 DMG"
echo "2. 测试 DMG 安装"
echo "3. 准备发布到 GitHub"
echo ""

# 清理测试构建
read -p "是否清理测试构建文件? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf build_test
    echo "✅ 已清理测试构建"
fi
