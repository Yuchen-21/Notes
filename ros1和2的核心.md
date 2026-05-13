# ros1和2的核心

## Topic：ROS1 最核心的发布订阅模型

发布者不关心谁订阅，订阅者也不直接关心谁发布。双方通过 Master 完成发现。
```bash
1. roscore 启动，Master 开始监听 XMLRPC

2. Publisher 节点启动
   它向 Master 注册：
   “我是 node_lidar，我发布 /scan，类型是 sensor_msgs/LaserScan”

3. Subscriber 节点启动
   它向 Master 注册：
   “我是 node_localization，我订阅 /scan”

4. Master 通知 subscriber：
   “现在有一个 publisher 在发布 /scan，它的 XMLRPC 地址是 xxx”

5. Subscriber 通过 XMLRPC 联系 Publisher：
   “我要订阅 /scan，你支持什么传输协议？”

6. Publisher 返回：
   “我支持 TCPROS，连接地址是 IP:PORT”

7. Subscriber 主动建立 TCP 连接

8. 双方交换 TCPROS header：
   topic、type、md5sum、callerid、message_definition 等

9. 校验通过后，Publisher 开始把序列化后的消息发给 Subscriber
```

    Publisher              Master               Subscriber
        │                    │                       │
        │ registerPublisher  │                       │
        │───────────────────►│                       │
        │                    │ registerSubscriber    │
        │                    │◄──────────────────────│
        │                    │                       │
        │                    │ publisherUpdate       │
        │                    │──────────────────────►│
        │                    │                       │
        │ requestTopic       │                       │
        │◄───────────────────────────────────────────│
        │                    │                       │
        │ TCPROS connect     │                       │
        │◄───────────────────────────────────────────│
        │                    │                       │
        │ serialized message │                       │
        │───────────────────────────────────────────►│
重点：
1. 也就是发现和数据传输是分离的。
2. 连接是由 subscriber 主动连 publisher。
3. 类型校验依赖 message type 和 md5sum。
4. 数据传输不是自描述完整对象，而是 ROS 消息序列化后的二进制流。

## 通信方式
默认TCP socket + ROS 自定义连接头 + ROS 消息序列化
也支持 UDPROS

## ROS1类似Qos的机制
1. queue_size
发布和订阅者都有应该根据实际情况设置，比如/cmd_vel订阅队列不应该太大，否则机器人可能在网络/计算卡顿后继续执行旧速度命令。
2. latch
```cpp
// 第三个参数为true时，publisher 保存最后一条消息，后来的 subscriber 一连接上就能收到这条旧消息。比较适合静态配置，初始化信息一类的状态。
ros::Publisher pub = nh.advertise<std_msgs::String>("/status", 1, true);
```
## Service 流程
1. Service server 注册 /reset_localization
2. Client 向 Master 查询 /reset_localization 由谁提供
3. Client 连接 server
4. 发送 request
5. server 返回 response

# ros1和2最大的区别
ROS1 是 Master 中心化发现 + TCPROS/UDPROS 点对点传输，通信策略比较固定，缺少标准 QoS。ROS2 底层基于 DDS/RMW，去掉了 ROS Master，发现机制由 DDS 完成，并且 QoS 成为通信接口的一部分，可以配置 reliability、durability、history、deadline、liveliness 等策略。ROS2 更适合多机器人、工业化、弱网络和复杂通信质量要求，但复杂度也更高。