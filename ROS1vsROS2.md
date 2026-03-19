# ROS1 vs ROS2
## 维护
1. ROS 1 的最后一个发行版 Noetic 已在二〇二五年五月三十一日达到官方 EOL（停止官方维护与安全更新），继续把 ROS 1 作为“主干中间件”将带来持续累积的安全与供应链风险。
2. ROS 2 在公开仓库下载占比与社区问题流量上已显著占优
《2025 ROS Metrics Report》给出二〇二五年十月 ROS 2 下载占比为 91.2%，并显示 Robotics Stack Exchange 上与 ROS 相关的问题中，“ros2”标签远多于“ros1”
3. 
```mermaid
flowchart LR
  subgraph ROS1["ROS 一 典型通信结构"]
    M["rosmaster / roscore\n(注册/发现 + 参数/日志等)"]
    N1["节点 A"]
    N2["节点 B"]
    N3["节点 C"]
    N1--"XMLRPC 注册/查询"-->M
    N2--"XMLRPC 注册/查询"-->M
    N3--"XMLRPC 注册/查询"-->M
    N1--"TCPROS/UDPROS P2P"---N2
    N2--"TCPROS/UDPROS P2P"---N3
  end

  subgraph ROS2["ROS 2 典型通信结构"]
    D["DDS/RTPS\n(分布式发现 + QoS + 安全插件)"]
    R1["节点 A\n(rclcpp/rclpy)"]
    R2["节点 B\n(rclcpp/rclpy)"]
    R3["节点 C\n(rclcpp/rclpy)"]
    R1---D
    R2---D
    R3---D
    R1--"发布/订阅\nQoS 契约"---R2
    R2--"发布/订阅\nQoS 契约"---R3
  end
```