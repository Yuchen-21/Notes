# ros2 笔记
## 概念
### Node（节点）
最重要的概念就是：

节点是 ROS 2 里的基本**功能**单元。

一个节点通常代表一个“独立职责”：

一个驱动节点
一个算法节点
一个可视化节点
一个控制节点

你可以先把它理解成：

**节点 = 一个进程里的一个功能角色 / 一个服务单元**
节点对外可以有：

topic：流式数据
service：请求响应
action：长时任务
parameter：节点配置

其中：
parameter 的远程访问，通常借助 service 实现
```
ROS 2 系统
│
├── Node（节点）: 功能单元
│   │
│   ├── Topic（话题）: 连续数据流
│   ├── Service（服务）: 一问一答
│   ├── Action（动作）: 长时间任务
│   ├── Parameter（参数）: 节点配置
│   └── Timer / Callback（定时器/回调）: 节点内部触发逻辑
│
├── Launch: 怎么把一堆节点一起启动
├── TF: 各坐标系之间的关系
├── QoS: 通信质量策略
├── Executor: 节点回调如何被调度执行
└── Package / Workspace: 代码怎么组织和构建
```
## 参数
参数被视为节点设置，参数通信机制是基于服务通信实现的。
把节点想成一台设备。

这台设备有：

实时上报数据口 —— topic
接受一次性控制命令的接口 —— service
执行长任务的接口 —— action
自己的配置菜单 —— parameter

而别人要远程查看或修改“配置菜单”，
底层是走一个“配置读写接口”，这接口实现方式像 service。

**Executor 决定节点里的回调是怎么跑起来的。**

## TF 机器人系统里的坐标关系树
```
map
 └── odom
      └── base_link
           ├── laser
           └── camera
```
## QoS 通信质量策略
QoS = Topic 通信的行为策略
## 整体
工作空间安排
```
ws_ros2/
├── src/
│   ├── pkg_a
│   ├── pkg_b
│   └── pkg_c
├── build/
├── install/
└── log/
```

```
                    +----------------------+
                    |      Launch          |
                    |  负责启动整套系统     |
                    +----------+-----------+
                               |
      ---------------------------------------------------------
      |            |              |              |            |
      v            v              v              v            v
+-----------+ +-----------+ +-----------+ +-----------+ +-----------+
| Camera    | | Lidar     | | SLAM      | | Planner   | | Controller|
| Node      | | Node      | | Node      | | Node      | | Node      |
+-----------+ +-----------+ +-----------+ +-----------+ +-----------+
     |              |             |              |             |
     | topic        | topic       | topic        | action      | topic
     v              v             v              v             v
 /image_raw       /scan         /map         /navigate     /cmd_vel

每个 Node 内部还有：
- parameter（配置）
- callback（回调）
- service（对外接口）
- executor 调度执行
```
节点内部结构
```
                +----------------------------------+
                |            Node                  |
                |----------------------------------|
                | name: /controller_node           |
                | parameters: max_speed, kp, kd    |
                |                                  |
                | publishers:   /cmd_vel           |
                | subscribers:  /odom, /path       |
                | services:     /reset_controller  |
                | actions:      （可选）           |
                | timers:       100Hz control loop |
                +----------------+-----------------+
                                 |
                           callbacks 被触发
                                 |
                            Executor 调度执行
```

## 机器人系统
应用/任务层        -> 导航、感知、任务编排、UI、日志、录包
系统集成层        -> 模块发现、消息交换、参数、launch、TF
设备/控制层        -> 电机驱动、总线通信、闭环控制、实时线程
ROS 2 在上两层通常很好用；但到了最底层控制闭环和硬实时设备层，你要非常谨慎。ROS 2 官方有单独的实时编程文档，说明实时场景需要额外考虑内存分配、调度和中间件行为。

ros2_control 也明确把自己定位成一个面向机器人控制、强调实时性能、硬件接口抽象和控制器管理的框架，这恰好说明“光靠普通 topic/service 把控制层全糊过去”并不是最佳实践。

```
[任务/应用层]
导航、任务编排、UI、日志、远程指令
=> 这里大量用 ROS 2，没问题

[系统集成层]
状态汇聚、参数、TF、launch、诊断、录包、仿真接口
=> 这里强烈建议 ROS 2

[算法层]
感知、定位、规划、决策
=> 算法本体做成普通库，外面套 ROS 2 node/component

[控制/设备层]
电机、驱动、总线、硬件抽象、实时循环
=> 不建议全靠普通 ROS 2 topic 硬顶
=> 用 ros2_control 或独立驱动框架，再暴露 ROS 2 接口
```

## 和阿波罗区别
阿波罗替换整个 ROS 2，在自动驾驶整车栈里是现实的；在通用机器人研发里，往往不如“ROS 2 做系统集成 + 局部高性能/高实时模块自定义化”更现实。

## 常用命令
colcon 是按工作区概念工作的。
它会默认找当前目录下的：
src/
build/
install/
log/
rm -rf build/ log/ install/
colcon build --packages-select cus_interfaces

## 自定义服务接口在不同工作空间下的使用
1. 同一个工作空间时
假设工作空间
/root/ws_ros2
├── src
│   ├── cus_interfaces
│   ├── demo_cpp_service
│   └── 其他包

CMakeLists.txt
find_package(cus_interfaces REQUIRED)
package.xml
<depend>cus_interfaces</depend>
实际要用的代码里
#include "cus_interfaces/srv/patrol.hpp"
cd /root/ws_ros2
colcon build
2. 不在同一个工作空间时
需要先把服务编译好，然后在其他工作空间里使用。
先 source A 的环境，再去编译B

## 工程
小项目 / 学习项目
接口包和业务包常常放一个工作空间，最省事。

稍正式一点
通常会单独拆一个接口包，比如：
my_robot_interfaces
cus_interfaces

因为这样：
消息/服务定义集中管理
业务包不会互相拷来拷去
不同模块都能依赖同一份接口
以后多工作空间 overlay 更清楚

## 快捷键设置
把下面这些加到的 ~/.bashrc：
```bash
alias croot='cd /root/ws_ros2'
alias cb='cd /root/ws_ros2 && colcon build'
alias cbs='cd /root/ws_ros2 && colcon build --packages-select'
alias cclean='cd /root/ws_ros2 && rm -rf build install log'
```
全部清理
cclean 
编一个包
cbs demo_cpp_service

## ros2 control 
核心不是“控制算法”，而是“硬件抽象 + 控制资源管理”

     URDF / <ros2_control> 描述硬件接口
          ↓
     Resource Manager 加载硬件插件
          ↓
     Controller Manager 加载控制器插件
          ↓
     read() 读取硬件状态
          ↓
     controller.update() 计算控制输出
          ↓
     write() 写入硬件命令

Controller Manager 定义为 ros2_control 的主组件，它负责管理控制器生命周期、访问硬件接口，并向 ROS 世界提供服务；其控制循环就是读取硬件、更新控制器、再写回硬件。Resource Manager 则负责抽象物理硬件和驱动，通过 pluginlib 加载硬件组件。
它提供低抖动控制循环和资源管理框架，但确定性取决于 RT kernel、线程调度、驱动 read/write 是否阻塞、controller update 是否实时安全。
先在 URDF 的 <ros2_control> 中声明硬件、关节、state interface、command interface；
然后实现 hardware component 插件，例如 SystemInterface；
在 read() 中读取编码器、IMU、电机状态；
在 write() 中下发速度、位置或力矩命令；
再通过 controller_manager 加载 joint_state_broadcaster、diff_drive_controller 或自定义 controller

## nav2 
Nav2 使用行为树来编排多个独立的模块化服务器；这些 task server 可以计算路径、控制努力、执行恢复行为，服务器之间通过 ROS action 或 service 与 BT 通信

     NavigateToPose Action
          ↓
     BT Navigator 行为树调度
          ↓
     Planner Server 生成全局路径
          ↓
     Smoother Server 可选平滑路径
          ↓
     Controller Server 跟踪路径，输出 cmd_vel
          ↓
     Behavior Server / Recovery 处理异常
          ↓
     底盘执行层

Nav2 解决的是
机器人从“我要去某个点”到“中途遇障、重规划、恢复、清 costmap、绕障、停靠、继续执行”的任务级导航编排问题。

     BT Navigator：任务大脑，决定什么时候规划、什么时候跟踪、什么时候恢复
     Planner Server：全局路径规划
     Controller Server：局部路径跟踪，输出速度命令
     Costmap：局部/全局环境代价地图
     Behavior Server：Spin、Backup、Wait、恢复行为等
     Lifecycle Manager：统一管理 Nav2 各节点生命周期
     TF：map / odom / base_link 等坐标关系基础


