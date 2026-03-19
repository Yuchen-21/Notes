# 机器人知识梳理
## 最小知识框架
1. 生产者负责放数据，不负责指定谁来处理。
2. 消费者不是等“通知”，而是等“条件成立”。
3. 队列存数据，条件变量负责唤醒等待线程。
4. 锁保护共享资源，但不要拿着锁做耗时工作。
5. 回调适合轻处理，重处理应该转到工作线程。 
6. 成熟的并发模型一定要考虑退出、超时、背压和异常，而不只是“能跑”。
系统几乎就是队列 + 锁 + 条件变量 + 状态机 + 线程边界
所以要打牢这几个基础，才能写好并发程序。
并发基础 + 线程模型 + 常见系统架构模型 + 工程设计意识
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