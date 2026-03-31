# ROS2 Docker


这个 OSRF overlay 仓库的 ROS 镜像是 amd64-only；
而你的 Orin 是 arm64，所以容器启动 /bin/sh 时直接 exec format error。
OSRF 自己的 docker_images 仓库里写得很清楚：Official Library 的 ros 镜像支持 amd64 和 arm64v8，但 OSRF Profile 里的 osrf/ros 只有 amd64。

## 代理
docker daemon 本身如果要走代理，要单独配 daemon proxy
假设你的代理是本机 127.0.0.1:7890，可以这样配：
sudo mkdir -p /etc/systemd/system/docker.service.d
sudo nano /etc/systemd/system/docker.service.d/http-proxy.conf
写入
[Service]
Environment="HTTP_PROXY=http://127.0.0.1:7890"
Environment="HTTPS_PROXY=http://127.0.0.1:7890"
Environment="NO_PROXY=localhost,127.0.0.1"


然后
sudo systemctl daemon-reload
sudo systemctl restart docker
systemctl show --property=Environment docker
最后docker compose build


## Dockerfile
FROM ros:humble-perception

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Singapore

RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    git \
    vim \
    nano \
    curl \
    wget \
    gdb \
    net-tools \
    iputils-ping \
    ros-humble-rviz2 \
    ros-humble-rqt \
    ros-humble-rqt-graph \
    && rm -rf /var/lib/apt/lists/*

RUN rosdep init || true
RUN rosdep update || true

WORKDIR /root/ws_ros2

RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc && \
    echo "if [ -f /root/ws_ros2/install/setup.bash ]; then source /root/ws_ros2/install/setup.bash; fi" >> /root/.bashrc

# 初始化 rosdep（首次可能已存在，失败可忽略）
RUN rosdep init || true
RUN rosdep update || true

WORKDIR /root/ws_ros2

RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc && \
    echo "if [ -f /root/ws_ros2/install/setup.bash ]; then source /root/ws_ros2/install/setup.bash; fi" >> /root/.bashrc

## docker-compose.yml

services:
  ros2_humble_dev:
    build: .
    container_name: ros2_humble_dev
    network_mode: host
    ipc: host
    pid: host
    stdin_open: true
    tty: true
    environment:
      - DISPLAY=${DISPLAY}
      - QT_X11_NO_MITSHM=1
      - XDG_RUNTIME_DIR=/tmp/runtime-root
    volumes:
      - /tmp/.X11-unix:/tmp/.X11-unix:rw
      - ./workspace:/root/ws_ros2
    working_dir: /root/ws_ros2

## 第一次启动容器
在 ~/ros2_humble_docker 下：
docker compose build
xhost +local:docker
docker compose up -d
docker exec -it ros2_humble_dev bash
进容器后：
source /opt/ros/humble/setup.bash
容器内
cd /root/ws_ros2
mkdir -p src
colcon build

## 日常流程
日常流程 1：启动环境
宿主机
```bash
cd ~/ros2_humble_docker
xhost +local:docker
docker compose up -d
进入容器
docker exec -it ros2_humble_dev bash
退出当前终端
exit
停掉容器
docker compose stop
再启动已经存在的容器
docker compose start
挂载目录
~/ros2_humble_docker/workspace/src
和 /root/ws_ros2/src 是同一个目录
加载环境变量
source /root/ws_ros2/install/setup.bash
关环境
如果只是临时退出容器：
exit
停掉并删除 compose 管理的容器/网络(会删掉容器的)
docker compose down

平时用 stop/start，少用 down/up。
```
