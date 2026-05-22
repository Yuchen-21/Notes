# Cyber Code Notes
## 遇到的问题

1. 组件在dag中可分为timer_component和component, timer_component是定时器组件, component是普通组件。前者才有interval属性。
2. node_ 没被教程特别强调，是因为它是 Component 框架替你管理的通信 handle。你写普通 binary 时要自己 CreateNode()；写 Component 时，mainboard 加载组件后，框架会根据 DAG 的组件名初始化内部 node_，并根据 readers 自动创建输入 Reader。你只需要在 Init() 里用 node_ 创建输出 Writer 或其它通信对象，在 Proc() 里处理输入消息。



## 源码阅读

### the first step:Component 加载与生命周期
cyber/component/component.h
cyber/component/component_base.h
cyber/mainboard/mainboard.cc
cyber/class_loader/
回答以下问题
    Component 是怎么注册的？
    CYBER_REGISTER_COMPONENT 做了什么？
    mainboard 怎么根据 DAG 找到 .so？
    mainboard 怎么根据 class_name 创建对象？
    Init() 什么时候调用？
    Proc() 是怎么被框架调用的？

### the second step: Reader / Writer / Node 通信抽象
cyber/node/node.h
cyber/node/node.cc
cyber/node/node_channel_impl.h
cyber/node/node_channel_impl.cc
cyber/node/reader.h
cyber/node/writer.h
回答
CreateReader 创建了什么？
CreateWriter 创建了什么？
Node 在 Cyber 里是不是类似 ROS NodeHandle？
Reader callback 和 Component Proc 有什么区别？
Writer::Write 后消息怎么进入 transport？

### the third step: 多输入融合与缓存
cyber/data/data_visitor.h
cyber/data/channel_buffer.h
cyber/data/cache_buffer.h
cyber/data/fusion/all_latest.h
cyber/data/fusion/data_fusion.h
回答
为什么第一个 reader 是主 channel？
其它 reader 的 latest 是怎么取的？
没有 latest 时为什么不进 Proc？
buffer 满了会怎样？
Proc 处理太慢会不会丢消息？
Cyber 默认融合和 TF 时间缓存有什么区别？

### the fourth step: 调度
重点
cyber/scheduler/
cyber/croutine/
cyber/task/
cyber/common/global_data.*

优先
cyber/scheduler/scheduler.cc
cyber/scheduler/policy/scheduler_classic.cc
cyber/scheduler/policy/scheduler_choreography.cc
cyber/croutine/croutine.h
cyber/croutine/routine_factory.h
回答
Proc 是在哪个线程执行的？
CRoutine 是怎么创建的？
Scheduler 怎么唤醒任务？
Processor 线程怎么创建？
classic 和 choreography 差异是什么？
CPU 亲和性在哪里设置？
### the fifth step: Transport 通信层
看
cyber/transport/
cyber/transport/dispatcher/
cyber/transport/receiver/
cyber/transport/transmitter/
cyber/transport/shm/
回答
同进程、跨进程、跨机器通信分别怎么走？
什么时候用共享内存？
Channel 消息怎么分发给 Reader？
高频点云为什么要考虑拷贝和队列积压？