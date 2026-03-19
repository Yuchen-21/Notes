# QT环境部署
以下为在orin agx ubuntu 20.04 arm64架构下部署QT环境的步骤，主要原因为履带车遥控器需要国产化，rk3588加上
## 安装QT
```bash
# 安装还是中科大的源方便，国外的源反而安装不上
./qt-online-installer-linux-arm64-4.10.0.run --mirror https://mirrors.ustc.edu.cn/qtproject
# 安装qt需要注册账号，我拿qq邮箱没收到验证码，用gmail收到了
# 填写完账号密码后，点击安装即可
qt选择需要安装的组件，我选择了以下：
QT 6.7.0 Desktop
QT Location(TP)
QT Positioning(TP)
QT WebEngine
# 安装路径我选择了固态硬盘中，QT目录
```
## 编译缺库
报错提示缺少 XKB（X Keyboard Extension），这是 Qt 用于处理键盘输入的核心库。之前可能装了运行库，但缺了开发头文件（-dev 包）。
```bash
sudo apt-get install build-essential libgl1-mesa-dev libxkbcommon-x11-0 libvulkan1 libxcb-xinerama0 -y
```
## 尝试编译然后发现位置找不到
然后一个个加，以下调试通过。
```bash
yyc_orin@ubuntu:~/extend/YYC/track_app_1_23/track_app/build$ cmake .. \
>   -DCMAKE_PREFIX_PATH=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64 \
>   -DQt6_DIR=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64/lib/cmake/Qt6 \
>   -DQt6QmlTools_DIR=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64/lib/cmake/Qt6QmlTools 
```
为了方便，我将以上命令添加到了bashrc中
```cpp
# 1. 让 CMake 自动找到 Qt 的根目录
export CMAKE_PREFIX_PATH=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64:$CMAKE_PREFIX_PATH

# 2. 强行指定 Qt6 和 QmlTools 的位置 (解决 CMake 3.16 老眼昏花的问题)
export Qt6_DIR=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64/lib/cmake/Qt6
export Qt6QmlTools_DIR=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64/lib/cmake/Qt6QmlTools

# 3. 把 qmake 加入命令行，方便查版本
export PATH=/home/yyc_orin/extend/Qt/6.7.0/gcc_arm64/bin:$PATH
```
## 地图
地图文件放在/userdata/map/目录下 记得原始文件位置使用绝对路径
```bash
ln -s 原始文件绝对路径 现在目标路径
ln -s /home/yyc_orin/extend/YYC/track_app_1_23/track_app/map/ /userdata/map/ 
```