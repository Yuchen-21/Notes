#!/bin/bash

# ================= 配置区域 =================
APP_NAME="./apptrack_app"
MALI_LIB_PATH="/usr/lib/aarch64-linux-gnu/libmali.so"
# ===========================================

# 获取脚本当前所在的绝对路径
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SHIM_DIR="$SCRIPT_DIR/mali_libs"

# 1. 【自动修复】检查并创建 Mali 库的“替身”
# 这样即使你删除了 build 目录，下次运行脚本也会自动重建链接，无需手动 ln
if [ ! -d "$SHIM_DIR" ]; then
    echo "[Init] Detected missing driver shims. Creating aliasing in $SHIM_DIR..."
    mkdir -p "$SHIM_DIR"
    ln -sf "$MALI_LIB_PATH" "$SHIM_DIR/libEGL.so"
    ln -sf "$MALI_LIB_PATH" "$SHIM_DIR/libEGL.so.1"
    ln -sf "$MALI_LIB_PATH" "$SHIM_DIR/libGLESv2.so"
    ln -sf "$MALI_LIB_PATH" "$SHIM_DIR/libGLESv2.so.2"
fi

# 2. 【环境劫持】设置环境变量
export LD_LIBRARY_PATH="$SHIM_DIR:$LD_LIBRARY_PATH"

# 3. 【Qt 配置】设置显示平台和渲染后端
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_INTEGRATION=eglfs_kms
export QSG_RHI_BACKEND=opengl

# 4. 【硬件兼容】解决 RK3588 特有的光标和色深问题
export QT_QPA_EGLFS_HIDDEN_CURSOR=1  # 禁用硬件光标防止崩溃
export QT_QPA_EGLFS_FORCE888=1       # 强制 24/32 位色深，防止渐变色断层

# 5. 【运行】启动程序
echo "[Run] Starting $APP_NAME with Mali GPU Acceleration..."

# 检查是否使用了 sudo (DRM 设备通常需要 root 权限)
if [ "$EUID" -ne 0 ]; then
  echo "Warning: run without sudo might cause 'Could not open DRM device' error."
fi

# 执行程序
"$SCRIPT_DIR/$APP_NAME"