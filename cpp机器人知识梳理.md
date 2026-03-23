# 机器人知识梳理
## 框架要练的是什么
- 能力抽象
- 接口契约设计
- 设备模型统一
- 适配层设计
- 解耦
- 状态管理
- 错误码与异常路径
- mock / testability 思维

不要再把自己定义成“给自研中间件填坑的人”，而要定义成“做设备能力抽象与中间层解耦的人”。
要把经历抽象成可迁移能力

你真正应该从这段经历里抽走的是：
如何把协议层封成能力层
如何定义设备抽象接口
如何设计统一消息模型
如何做适配层
如何在接口不稳定时保住上层业务
如何为节点转发设计 mock 接口与测试桩
如何让中间件解耦运动控制业务

答：
- 我之前做过一段比较偏中间层和架构演进的工作，不只是写节点功能，而是在尝试把系统从协议驱动逐步往设备能力驱动演进。
- 我的理解是，底层协议层主要解决“怎么通信”，但上层业务真正需要的是稳定设备能力，而不是直接面对字段和报文。所以我比较关注几件事：第一，怎么把底层协议细节收敛在适配层里；第二，怎么围绕设备状态、控制、健康等能力定义更稳定的抽象接口；第三，怎么建立统一消息模型，让上层节点围绕统一语义写逻辑，而不是到处做设备特判。
- 另外我感受也比较深的是，在底层接口不稳定的时候，不能让上层业务直接暴露在变化面前，所以我会倾向在中间加稳定边界，尽量把变化消化在适配层内部。包括像节点转发这类逻辑，我也会更关注可测试性，希望通过抽象接口、mock 实现和测试桩，让很多逻辑在没有真实设备的情况下也能验证。
总体上，我会把中间件理解为通用基础设施，把运动控制理解为业务逻辑，两者边界要尽量清楚，这样系统才更好演进，也更容易维护和测试。
## 学习计划
第 1 周
mutex / lock_guard / unique_lock
condition_variable
单生产者单消费者 BlockingQueue
第 2 周
多生产者多消费者
close / timeout / 优雅退出
异步日志小 demo
第 3 周
线程池基础
状态机基础
流水线与模块解耦思维
第 4 周
ROS2 概念接入
executor / callback group 直觉
回调入队、worker 处理模式
用 top/htop 看自己 demo
## 最小知识框架
1. 生产者负责放数据，不负责指定谁来处理。
2. 消费者不是等“通知”，而是等“条件成立”。
3. 队列存数据，条件变量负责唤醒等待线程。
4. 锁保护共享资源，但不要拿着锁做耗时工作。
5. 回调适合轻处理，重处理应该转到工作线程。 
6. 成熟的并发模型一定要考虑退出、超时、背压和异常，而不只是“能跑”。
系统几乎就是队列 + 锁 + 条件变量 + 状态机 + 线程边界
所以要打牢这几个基础，才能写好并发程序。
**并发基础 + 线程模型 + 常见系统架构模型 + 工程设计意识**
## 关于锁
std::mutex 就是一把互斥锁。
它本身像“门锁”，你用它来保护共享资源。
```cpp
std::mutex mtx;
mtx.lock();
// 访问共享数据
mtx.unlock();
```
但这种写法不安全。
因为中间一旦 return、throw、或者代码路径复杂，就可能忘记 unlock。
### RAII是什么  RYE" (ral)
Resource Acquisition Is Initialization
(RAII) is a C++ programming technique that binds the lifecycle of resources (memory, file handles, locks) to object lifetime. Resources are acquired in the constructor and automatically released in the destructor when the object goes out of scope. This ensures exception safety and prevents leaks.
### lock guard 
```cpp
{
    std::lock_guard<std::mutex> lock(mtx);
    // 访问共享数据
}
// 出作用域自动解锁
优点：
1. 简单
2. 安全
3. 不容易忘解锁
缺点：
1. 不够灵活
2. 不能中途解锁
3. 不能配合 condition_variable::wait
```
### unique_lock
它也能自动解锁，但支持：
1. 延迟加锁
2. 中途解锁
3. 重新加锁
4. 所有权移动
```cpp
std::unique_lock<std::mutex> lock(mtx);
// 做一些事
lock.unlock();
// 做不需要锁的耗时操作
lock.lock();
```
配合条件变量等待
普通临界区保护：优先 lock_guard
需要 wait / unlock / lock 灵活控制：用 unique_lock
## notify_one 和 notify_all
notify_one()：叫醒一个等待线程
notify_all()：叫醒所有等待线程
常见用法：
新任务来了：通常 notify_one
系统关闭了：通常 notify_all
## 完整练习链
第一步：单生产者 + 单消费者
第二步：单生产者 + 多消费者
第三步：多生产者 + 单消费者
第四步：多生产者 + 多消费者
第五步：加 close
第六步：加超时 pop
第七步：加容量上限