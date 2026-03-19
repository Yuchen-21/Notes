# posix_study
## 1. 概念
可移植操作系统接口,POSIX,是IEEE 1003.1的缩写，定义了操作系统的接口标准。进程间通信 (IPC) 机制。
POSIX 消息队列是一种遵循 POSIX 标准 (IEEE Std 1003.1-2001 及更高版本) 的进程间通信 (IPC) 机制，它允许不相关联的进程（甚至是同一进程内的线程）通过队列传递结构化的数据（消息）。

1. 消息： 传输的基本单位。每个消息是一个字节序列（本质上是一个字节数组），带有一个整型的优先级 (从 0 到 sysconf(_SC_MQ_PRIO_MAX) - 1，通常至少为 32，0 表示最低优先级)。
2. 队列： 消息的容器。队列有名称（以 / 开头的字符串，如 /my_queue），进程通过这些名称访问队列。队列的生命周期独立于创建它的进程（除非显式删除）。
3. 生产者/发送者： 向队列中写入消息的进程或线程。
4. 消费者/接收者： 从队列中读取消息的进程或线程。

## 2.主要api函数
**需要包含 <mqueue.h> 和链接 -lrt 库**

```CPP
    mqd_t mq_open(const char *name, int oflag, ... /* mode_t mode, struct mq_attr *attr */);

        功能： 创建或打开一个现有的消息队列。
        参数：
            name: 队列名称，必须以 / 开头（Linux 实现中通常挂载在 /dev/mqueue）。
            oflag: 标志位，指定打开方式：
                O_RDONLY: 只读 (接收)。
                O_WRONLY: 只写 (发送)。
                O_RDWR: 读写。
                O_CREAT: 如果队列不存在则创建。
                O_EXCL: 与 O_CREAT 一起使用，确保创建新队列（如果队列已存在则失败）。
                O_NONBLOCK: 以非阻塞模式打开队列（后续 mq_send, mq_receive 不会阻塞）。
            mode (可选，当 O_CREAT 时有效): 权限位 (类似文件权限，如 0666)。
            attr (可选，当 O_CREAT 时有效): 指向 struct mq_attr 的指针，用于设置新队列的属性 (mq_maxmsg, mq_msgsize)。如果为 NULL，则使用系统默认值（可通过 /proc 文件系统查看调整）。
        返回值： 成功返回消息队列描述符 (mqd_t)，失败返回 (mqd_t) -1 并设置 errno。
```
```cpp
   int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        功能： 向打开的队列发送一条消息。
        参数：
            mqdes: mq_open 返回的描述符。
            msg_ptr: 指向要发送消息数据的指针。
            msg_len: 消息数据的长度（字节数）。必须小于等于创建队列时指定的 mq_msgsize。
            msg_prio: 消息的优先级 (0 最低优先级)。
        阻塞行为： 如果队列已满 (mq_curmsgs == mq_maxmsg)：
            默认 (O_NONBLOCK 未设置)：阻塞调用线程，直到队列有空间。
            设置了 O_NONBLOCK: 立即返回失败，errno = EAGAIN。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno。
```
 
```CPP
    ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        功能： 从打开的队列接收一条消息（总是接收队列中最高优先级里最早的消息）。
        参数：
            mqdes: mq_open 返回的描述符。
            msg_ptr: 指向用于存放接收到的消息数据的缓冲区指针。
            msg_len: 缓冲区的大小（字节数）。必须大于等于创建队列时指定的 mq_msgsize，否则 mq_receive 会失败 (EMSGSIZE)。
            msg_prio: (输出参数) 指向 unsigned int 的指针，用于接收消息的优先级。如果不需要优先级，可以设为 NULL。
        阻塞行为： 如果队列为空：
            默认 (O_NONBLOCK 未设置)：阻塞调用线程，直到队列中有消息。
            设置了 O_NONBLOCK: 立即返回失败，errno = EAGAIN。
        返回值： 成功返回接收到的消息的字节数，失败返回 -1 并设置 errno。注意返回值 0 是合法的（表示收到了一条长度为 0 的消息）。
```
```CPP
    int mq_close(mqd_t mqdes);
        功能： 关闭进程打开的消息队列描述符。类似于 close 文件描述符。关闭描述符并不会删除队列本身。进程终止时会自动关闭其打开的所有消息队列描述符。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno。
```
```cpp
    int mq_unlink(const char *name);
        功能： 删除一个消息队列的名字。队列本身只有在所有打开它的描述符都关闭后 (mq_close) 才会被系统销毁。类似于 unlink 文件。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno（常见错误 ENOENT 表示名字不存在）。
```
```cpp
    int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
        功能： 获取指定队列的当前属性 (mq_flags, mq_maxmsg, mq_msgsize, mq_curmsgs)，填充到 attr 指向的结构中。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno。
```
```cpp
    int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr);
        功能： 设置指定队列的属性。只能设置 mq_flags 中的 O_NONBLOCK 标志位！mq_maxmsg 和 mq_msgsize 在队列创建后是不可修改的。
        参数：
            newattr: 指向包含新设置的结构体指针 (主要设置 mq_flags)。
            oldattr: 如果非 NULL，用于存放设置前的旧属性。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno。
```
```cpp
    int mq_notify(mqd_t mqdes, const struct sigevent *notification);
        功能： 注册或注销异步通知。当空队列接收到一条新消息时，通知注册的进程（通过信号或创建线程）。
        参数：
            mqdes: 队列描述符。
            notification: 指向 struct sigevent 的指针，定义通知方式。常用方式：
                SIGEV_SIGNAL: 发送一个信号 (sigev_signo)。
                SIGEV_THREAD: 创建一个新线程执行指定的函数 (sigev_notify_function)。
            设为 NULL 表示注销当前通知。
        重要： 通知是一次性的。收到通知并处理消息后，如果需要继续接收通知，必须再次调用 mq_notify 重新注册。
        返回值： 成功返回 0, 失败返回 -1 并设置 errno。
```

## 3.工作流程示例：
### 3.1 创建队列 (Producer)：
```C++
    #include <mqueue.h>
    #include <fcntl.h> // For O_* constants

    #define QUEUE_NAME "/my_test_queue"
    #define MAX_MSG_SIZE 1024
    #define MAX_MSG_NUM 10

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG_NUM;
    attr.mq_msgsize = MAX_MSG_SIZE;

    mqd_t mq_fd = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq_fd == (mqd_t)-1) {
        perror("mq_open (create)");
        exit(EXIT_FAILURE);
    }
```

### 3.2 发送消息 (Producer)：

```cpp
    char msg_text[] = "Hello from Producer!";
    unsigned int prio = 10; // Priority

    if (mq_send(mq_fd, msg_text, strlen(msg_text) + 1, prio) == -1) { // +1 for null terminator (if needed)
        perror("mq_send");
        // Handle error (e.g., queue full - EAGAIN if nonblocking, or EMSGSIZE if too big)
    }
```
### 3.3 打开队列 (Consumer)：

```cpp
    mqd_t mq_fd = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq_fd == (mqd_t)-1) {
        perror("mq_open (open)");
        exit(EXIT_FAILURE);
    }
```
### 3.4 接收消息 (Consumer)：

```cpp
    char buffer[MAX_MSG_SIZE + 1]; // Ensure buffer is large enough
    unsigned int recv_prio;

    ssize_t bytes_read = mq_receive(mq_fd, buffer, sizeof(buffer), &recv_prio);
    if (bytes_read == -1) {
        perror("mq_receive");
        // Handle error (e.g., queue empty - EAGAIN if nonblocking)
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0'; // Null-terminate if treating as string (optional)
    printf("Received (%u): %s\n", recv_prio, buffer);
```
### 3.5 关闭队列描述符 (Both)：
```cpp
    if (mq_close(mq_fd) == -1) {
        perror("mq_close");
    }
```
### 3.6 删除队列 (Producer or Cleanup Task)：


```Cpp
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("mq_unlink");
    }
```

### 3.7 关键点 & 注意事项：

**Linux 实现**： 在 Linux 上，POSIX 消息队列通常通过挂载一个虚拟文件系统 (mqueue) 来实现。你可以使用命令 mount -t mqueue none /dev/mqueue 将其挂载到 /dev/mqueue (可能需要 root)，然后像查看文件一样查看队列属性 (ls -l /dev/mqueue, cat /dev/mqueue/myqueue)。
**资源限制**： 系统对消息队列的总数、单个进程可打开的描述符数、队列的总存储空间有限制（可通过 /proc/sys/fs/mqueue/ 下的文件查看和调整）。
**消息边界**： 接收方总是收到一条完整的消息（发送方 mq_send 发送的整个 msg_len 字节）。这是它与管道/FIFO/Sockets 流式 IPC 的关键区别之一。(ps:就是发可以断断续续，接收方去取是按照完整消息取的)
**非阻塞模式**： 使用 O_NONBLOCK 或 mq_setattr 设置非阻塞标志后，mq_send 在队列满时和 mq_receive 在队列空时会立即返回 EAGAIN 错误。这通常用于轮询或与其他 I/O 复用机制（如 poll, select, epoll）配合使用。注意：mqd_t 本身通常不能直接用于 poll/select/epoll。通常的做法是结合 mq_notify (例如使用 SIGEV_THREAD 创建一个管道或 eventfd，然后监视那个描述符)。
**异步通知 (mq_notify)**： 这是避免轮询队列空状态的高效方式。务必记住它是一次性的，处理完通知后需要重新注册。SIGEV_THREAD 方式通常比 SIGEV_SIGNAL 更灵活、更安全（避免信号处理函数的限制）。
**优先级**： 善用优先级可以处理紧急消息。
**错误处理**： 至关重要！ 务必检查每个 API 调用的返回值 (-1) 并处理 errno (perror, strerror)。常见错误包括 EACCES (权限不足)、EEXIST (O_CREAT | O_EXCL 冲突)、ENOENT (打开不存在的队列)、EAGAIN (非阻塞模式下的满/空)、EMSGSIZE (消息过大)、ENOSPC (系统队列资源耗尽)。

应用场景：

    解耦组件： 生产者和消费者独立运行，不需要知道对方的存在或状态。
    流量削峰： 当生产者速度远超消费者时，队列作为缓冲区积累消息，避免压垮消费者。
    异步处理： 生产者发送消息后可以立即返回，消费者异步处理。
    任务分发/工作队列： 多个消费者从一个队列中取出任务执行。
    日志聚合： 多个进程将日志发送到中心队列，由专门的日志处理器消费写入文件或数据库。
    事件通知： 通知其他进程发生了特定事件（带或不带数据）。

对比其他 IPC：

    管道/FIFO： 字节流，无消息边界，无优先级，通常用于父子进程或关联进程。
    System V消息队列 (msgget, msgsnd, msgrcv)： 功能类似 POSIX MQ，但 API 风格较老、较复杂。POSIX MQ 通常更简洁、功能更丰富（如异步通知）。
    Sockets： 功能强大（跨机器），但本机通信开销相对较大，配置相对复杂。
    共享内存： 速度最快，但需要复杂的同步机制（信号量、互斥锁）。

总结：

POSIX 消息队列提供了一种标准化的、基于消息的、支持优先级的、可靠的进程间通信方式，特别适合于需要解耦、异步处理、流量控制和优先级调度的场景。理解其核心概念、API 函数、阻塞/非阻塞行为、异步通知机制以及资源限制是有效使用它的关键。在实际项目中，结合 mq_notify（尤其是 SIGEV_THREAD）和 I/O 复用技术可以构建高效的事件驱动 IPC 方案。记得始终进行严格的错误处理！

## 虚函数

指向基类的指针在操作它的多态类对象时，会根据不同的类对象，调用其相应的函数，这个函数就是虚函数。
#  信号量学习
