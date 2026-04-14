# FoxGlove-Studio调研
[TOC]
## 1. foxglove studio 介绍
foxglove 是一个曾经开源的可视化和调试工具，提供多种方式使用。Foxglove可在 Windows、macOS 和 Linux 系统上运行，既有桌面应用程序版本，也有网页浏览器版本。这使用户无需安装软件即可灵活地在本地或远程工作。浏览器版本支持多人协作，允许团队通过任何联网设备安全地共享布局并实时传输数据。
最初的 Foxglove Studio 是开源项目（MPL 2.0），在2024年7月官方明确做了停止开源版本，把 Studio + Data Platform 合并成一个商业产品，现在的实际状态为Open-core（开放内核） + 商业闭源增强。作为一个商业化的机器人数据观测平台核心 UI 与平台能力已闭源。

### 1.1 价格区别
提供免费套餐，免费用户支持到3个用户，最多10个设备，以及10GB的云空间。
（其他主要是团队协作方面的功能，对于数据可视化及记录是免费的）
### 1.2 foxglove 主要功能亮点
Foxglove拥有现代化的友好用户界面，配备灵活的仪表盘和响应迅速的控件。
它的设计旨在方便所有经验水平的用户使用，使他们能够更轻松地**探索、分析和共享机器人数据**。
该界面大量采用图形元素，而非命令或配置文件，从而降低了不熟悉 ROS 或机器人工具的用户的入门门槛。
## 2. foxglove 与 RViz 1/2 对比

1) 实时 ROS 连接。

- Foxglove：通过 foxglove_bridge (WebSocket) 连接 ROS 1，通过 SDK 连接 ROS 2。可在桌面或浏览器上运行，适合远程评审。
- RViz：与 ROS 集成度最高，是本地调试的理想选择。交互式标记支持在 RViz 内直接进行 3D 操作。

2) 文件播放和格式。

- Foxglove：支持拖放 ROS 1 .bag、ROS 2 .db3 和 MCAP 文件。可将多个文件合并为一个时间线。对于旧版 .db3 文件，Foxglove 建议转换为 MCAP 格式以获得独立的元数据。
背景：ROS 2 Iron 将 MCAP 设置为 rosbag2 的默认存储格式，简化了跨工具工作流程。

3) 3D 和交互模型。

- RViz：交互式标记（移动/旋转/上下文菜单）功能强大且原生，支持循环内操作。
- Foxglove：采用同步面板的现代化可视化方案；团队通常通过发布/远程操作和丰富面板而非 RViz 风格的标记基元来处理交互。

4) 协作与扩展。

- Foxglove：专为团队打造——布局、设备、事件以及用于浏览大型数据集的快速时间线。免费方案：3 个用户/10 台设备/10GB 云空间。

5) 可扩展性和 SDK。

- Foxglove：提供用于自定义面板/转换器的扩展程序（JS/TS）和新的数据加载器扩展程序；Foxglove SDK（C++/Py/Rust，MIT 许可）用于流式传输或记录到 MCAP。

- RViz：C++ 插件系统；拥有庞大的显示和工具社区。
## 3.迁移快速入门（RViz → Foxglove）

开放数据：拖放 .bag/.db3/.mcap 文件——或将多个文件合并到一个时间线中加载。
实时连接：运行 foxglove_bridge（ROS 1）SDK（ROS 2），并在 Foxglove 中打开 WebSocket。

添加上下文：按设备组织、标注事件，并通过时间线进行浏览。

程序化数据摄取：使用 Foxglove SDK（C++/Python/Rust）进行实时流式传输或将日志记录到 MCAP。
## 4.替代性分析
问：Foxglove 可以完全替代 RViz 吗？
（foxglove官方问答）
答：对**于许多团队工作流程（文件分析、远程评审、共享布局），可以。
但对于 ROS 内部的交互式标记操作，RViz 仍然表现出色。**

问：Foxglove 可以打开 ROS 2 的 .db3 和 MCAP 文件吗？
（foxglove官方问答）
答：可以——支持 .db3 和 .mcap 文件；Foxglove 建议使用 MCAP 以确保可移植性。

问：如何将 ROS 2 连接到 Foxglove？
答：使用 foxglove_bridge 可以建立稳定、高性能的 WebSocket 连接。  
## 5. 替换RViz方案
考虑到Foxglove的官方推荐，替代主要分两种情况，在线和离线。
在线主要考虑的是实时接收传感器数据，离线主要考虑的是播放记录的数据。
### 5.1 在线方案
```
传感器驱动（C++）
        ↓
CyberRT（发布订阅）
        ↓
Bridge （自研开发）
        ↓
Foxglove（WebSocket）
        ↓
Foxglove Studio (可视化)
```
待开发内容：
- 传感器驱动节点，需要sdk基于cpp自己开发，主要负责接收传感器数据并发布到CyberRT。每个类型传感器需要单独开发。

- Bridge 需要订阅 CyberRT 发布的传感器数据 topic。然后转换成Foxglove schema格式，再通过WebSocket发送到Foxglove Studio。

支持的格式主要有以下几种：

    Protobuf
    JSON Schema
    ROS 1
    ROS 2
    TypeScript
    FlatBuffers

proto可视化标准可以参考https://github.com/foxglove/foxglove-sdk/tree/main/schemas/proto/foxglove

Schema 可以在官网https://docs.foxglove.dev/docs/sdk/schemas 查询
下面列举pointcloud点云数据的schema：

field | type | description
--- | --- | ---
timestamp | Timestamp |Timestamp of point cloud
frame_id | string |Frame of reference frame
pose | Pose |The origin of the point cloud relative to the frame of reference
point_stride | uint32 |Number of bytes between points in the data
fields |PackedElementField[]|Fields in data. At least 2 coordinate fields from x, y, and z are required for each point's position; red, green, blue, and alpha are optional for customizing each point's color.
data|bytes|Point data, interpreted using fields

Foxglove 用的是“结构描述 + 原始内存”模式，类似sensor_msgs/PointCloud2。
#### 5.1.1 替换的schema分类：
以下按照自动驾驶和机械臂控制，替换RViz需要分层替换：
1. **传感器层(Sensor Layer)**

数据|Rviz|Foxglove|优先级
--- | --- | --- | ---
激光雷达 | PointCloud2|foxglove.PointCloud|高
相机图像 | Image|foxglove.Image|高
IMU | Imu|foxglove.Vector3或foxglove.Velocity3|高
2D雷达 | LaserScan|foxglove.LaserScan |低  

2. **感知层(Perception)**

数据|Rviz|Foxglove|必要性|说明
--- | --- | --- | ---| ---
检测框 |Marker|foxglove.SceneUpdate|高|画3D框
点云叠加 |PointCloud2|foxglove.PointCloud|高|雷达叠加
分割结果|	Marker	|foxglove.SceneUpdate|	中 |可行驶区域分割

3. **定位层（Localization）**

数据|Rviz|Foxglove|必要性|说明
--- | --- | --- | ---| ---
车辆位姿 |Pose|foxglove.Pose / PoseStamped|高| 姿态
TF树 |FrameTransforms|foxglove.FrameTransforms|高| 相对位置变换

4. **地图层（Mapping）**

数据|Rviz|	Foxglove|必要性|说明
--- | --- | --- | ---| ---
占据栅格|	OccupancyGrid	|foxglove.Grid	|低| 2D地图栅格
HD Map|	Marker	|foxglove.SceneUpdate	|中|全局地图

5. **规划层（Planning）**

数据|Rviz|	Foxglove|必要性|说明
--- | --- | --- | ---| ---
轨迹 |	Path	|foxglove.LineStrip		|高| 轨迹可视化
目标点	|Marker	|foxglove.SceneUpdate	|高|点击导航

6. **控制层（Control）**
控制层在 RViz 和 Foxglove 中并不属于空间可视化对象，而是系统调试信息层，用于展示控制器输出状态、车辆运行参数及误差等信息。在 Foxglove 中通常通过 JSON 或 Plot 面板进行展示，而非标准几何 schema。（必要性中）

7. **机器人/机械臂（扩展）**

数据|	RViz|	Foxglove Schema|必要性|说明
--- | --- | --- | ---| ---
机器人模型|	URDF	|URDF + TF	|高|	模型显示
关节状态|	JointState	|JointState	|高|	关节姿态，包括力和扭矩
末端位姿|	Pose|	foxglove.Pose	|中|	可选
8. **调试层（Debug）**
标记、日志和曲线数据都为可选内容，根据需要选择是否增加开发。

#### 5.1.2 在线远程方案
foxglove stuidio主要是通过WebSocket接收数据，然后可视化展示。对原生的ROS1和ROS2都有支持。ROS1 需要安装foxglove_bridge包。
**ROS1:**
```bash
sudo apt install ros-$ROS_DISTRO-foxglove-bridge
roslaunch --screen foxglove_bridge foxglove_bridge.launch
```
**ROS2:**
```bash
sudo apt install ros-$ROS_DISTRO-foxglove-bridge
ros2 launch foxglove_bridge foxglove_bridge_launch.xml
```
安装并启动后，通过websocket连接到foxglove_bridge节点，即可接收数据并可视化展示在桌面端或者网页端。
当运行在本地时连接地址为ws://localhost:8765，当机器人远程时连接地址为ws://ROBOT_IP:8765
###### 5.1.2.1 带宽
连接主要依靠websocket建立，所以网络带宽需要纳入考虑，避免因为网络问题导致数据丢失、卡顿延迟等。

现在履带车没有实时建图的需求和业务，主要基于实时点云信息进行定位和导航。高精地图（HD Map）提前做，在线是定位的话，不做实时建图传输，所以几乎没有“SLAM地图带宽问题”。一般只有机器人做室内业务比如扫地机 / AMR自主移动机器人才会需要考虑实时构建 occupancy grid 才会传 map。
###### 5.1.2.2 雷达带宽
点云是带宽最大头，对于不同地图形式会占不同的带宽。
Occupancy Grid 对应 foxglove.Grid，Scene形式地图对应foxglove.SceneUpdate，点云地图对应foxglove.PointCloud，压缩地图对应foxglove.CompressedPointCloud。
对于原始点云数据，raw PointCloud 
32线 vs 64线（10Hz）
雷达|	带宽
--- | ---
32线|	3–5 MB/s
64线|	6–15 MB/s
128线|	15–40 MB/s

32线10Hz雷达在未压缩情况下点云带宽约3–8MB/s，工程中经过滤波与压缩后通常可降至3–5MB/s，是自动驾驶远程可视化的主要数据源之一。点云压缩与裁剪在点云数据待开发。
###### 5.1.2.3 相机带宽
按照 1080p RGB 30fps 计算
编码|	带宽
--- | ---
RAW RGB|	~180 MB/s
1080p YUV422 |	~119 MB/s MB/s
MJPEG|	~20–60 MB/s
H264（主流）|	~3–10 MB/s
H265（优化）|	~2–6 MB/s

大部分都需要压缩，实际工程大概都处于3-10mb/s之间。
###### 5.1.2.4 带宽需求
主动驾驶主要带宽需求来源如下
点云 + 图像 + TF + Pose + 规划轨迹，除了图像和点云以外剩下的都小于1mb/s。
基于履带车，工业可用系统（压缩后）总体大概需要7-16MB/s的带宽。这是只考虑一个雷达和一个相机的情况。
手机热点大概可以支持“轻量自动驾驶远程调试”，但无法支持“全量传感器原始流”，4G: 1–4 MB/s，5G: 2–12 MB/s，可以设置分级传输速率策略。
能满足10–15 MB/s，优秀的5G环境:
**Full mode（电台 / 5G好）**
- 1080p H264
- 压缩点云
- 全TF

能满足4–8 MB/s，普通热点环境:
**Stable mode（普通热点）**
- 720p H264
- 降采样点云
- TF降频

<1 MB/s,比如弱网环境:
**Emergency mode（差网）**
Pose + trajectory
少量 marker

### 5.2 离线方案
离线主要是对比rosbag包的记录和播放。
Foxglove 主要支持rosbag包和MCAP的播放。MCAP本质是通用“传感器数据录制容器格式”，类似ROS的rosbag，CyberRT的Record文件。主要包含
1. topic name
2. timestamp
3. message payload（protobuf/json/bytes）
现在替换主要MCAP writer，可以有两种开发方式，1是用 Foxglove SDK中的 MCAP writer API ，2是自己写 MCAP（不推荐）要实现：MCAP header、chunk encoding、message index、schema index
也就是流程是
```bash
CyberRT
   ↓
Bridge（上文有提及）
   ↓
Foxglove schema（proto）
   ↓
MCAP writer（SDK）
   ↓
MCAP file
   ↓
Foxglove Studio playback
```
与在线不同的是，离线方案可以不考虑网络带宽，所以MCAP可以存“更完整数据”，甚至raw PointCloud。
开发难点主要有：
1. 时间戳同步统一
2. schema注册 每个topic必须有：schema name、schema definition
3. Foxglove 只要 flatten，也就是一整段连续内存（byte array），不能有分块存储。1. 点云 = GPU buffer（必须 flatten）2. 图像 = 编码流（JPEG/H264）3. TF = 坐标树（frame graph）