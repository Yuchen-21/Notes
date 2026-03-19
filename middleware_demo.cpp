#include <iostream>
#include <vector>
#include <memory>   // 智能指针核心头文件
#include <unordered_map>
#include <string>
#include <cstring>  // for memcpy

// ==========================================
// Part 1: 数据载体 (模拟原始字节流)
// ==========================================

// 定义消息类型常量
const uint32_t TYPE_LCM_DOG_CTRL = 0x01;
const uint32_t TYPE_ROS_LIDAR    = 0x02;

struct RawMessage {
    uint32_t type_id;
    std::vector<uint8_t> buffer;
    
    // 构造函数：打印日志是为了让你看到内存什么时候被分配
    RawMessage(uint32_t type, const std::string& data) : type_id(type) {
        // 模拟把数据放入 buffer
        buffer.assign(data.begin(), data.end());
        std::cout << "  [资源] RawMessage (Type " << type << ") 被创建在地址: " << this << std::endl;
    }

    // 析构函数：关键！观察这里，看看它什么时候被自动调用
    ~RawMessage() {
        std::cout << "  [资源] RawMessage (Type " << type_id << ") 被销毁 (内存释放)" << std::endl;
    }
};

// 【关键点1】定义别名，以后代码里只传 MsgPtr，不传 RawMessage*
using MsgPtr = std::shared_ptr<RawMessage>;


// ==========================================
// Part 2: 业务逻辑接口 (替代 switch-case 的 case 内容)
// ==========================================

// 抽象基类
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default; // 虚析构函数必不可少
    
    // 纯虚接口：子类必须实现
    // 注意：这里接收 MsgPtr，引用计数会 +1
    virtual void process(MsgPtr msg) = 0;
};

// 具体业务1：处理机器狗 LCM 控制指令
class LcmDogHandler : public IMessageHandler {
public:
    void process(MsgPtr msg) override {
        // 模拟解码
        std::string cmd(msg->buffer.begin(), msg->buffer.end());
        std::cout << "    -> [LCM处理] 解码指令: " << cmd << " | 引用计数: " << msg.use_count() << std::endl;
        
        // 模拟业务判断
        if (cmd == "RUN") {
            std::cout << "    -> [LCM处理] 机器狗正在奔跑..." << std::endl;
        }
    }
};

// 具体业务2：处理 ROS 雷达数据
class RosLidarHandler : public IMessageHandler {
public:
    void process(MsgPtr msg) override {
        // 模拟复杂耗时操作
        std::cout << "    -> [ROS处理] 正在转换点云数据... 数据大小: " << msg->buffer.size() << std::endl;
    }
};


// ==========================================
// Part 3: 核心分发器 (替代 switch-case 结构)
// ==========================================

class MessageDispatcher {
private:
    // 【关键点2】所有权独占。Dispatcher 拥有这些 Handler，Dispatcher 销毁，Handler 也销毁。
    std::unordered_map<uint32_t, std::unique_ptr<IMessageHandler>> handlers_;

public:
    // 注册 Handler
    void registerHandler(uint32_t type, std::unique_ptr<IMessageHandler> handler) {
        // std::move 是必须的，因为 unique_ptr 不能被拷贝，只能被转移
        handlers_[type] = std::move(handler);
    }

    // 分发消息
    void dispatch(MsgPtr msg) {
        // 以前这里是 switch(msg->type_id) ...
        auto it = handlers_.find(msg->type_id);
        if (it != handlers_.end()) {
            // 找到了对应的处理器，调用接口
            it->second->process(msg);
        } else {
            std::cout << "    -> [警告] 未知的消息类型: " << msg->type_id << std::endl;
        }
    }
};


// ==========================================
// Part 4: 模拟运行环境
// ==========================================

// 模拟驱动层接收数据
MsgPtr driverReceiveData(uint32_t type, std::string content) {
    std::cout << "\n[驱动层] 收到硬件中断数据..." << std::endl;
    
    // 【关键点3】使用 make_shared 创建对象
    // 这行代码执行完，堆内存分配，引用计数 = 1
    return std::make_shared<RawMessage>(type, content);
}

int main() {
    // 1. 系统初始化
    std::cout << "=== 系统启动: 初始化中间件 ===" << std::endl;
    MessageDispatcher dispatcher;

    // 注册业务逻辑 (这是你在初始化阶段做的)
    // 这里的 make_unique 建立了 Handler 对象，并移交给 dispatcher 管理
    dispatcher.registerHandler(TYPE_LCM_DOG_CTRL, std::make_unique<LcmDogHandler>());
    dispatcher.registerHandler(TYPE_ROS_LIDAR, std::make_unique<RosLidarHandler>());

    std::cout << "=== 系统运行: 开始处理消息流 ===" << std::endl;

    // --- 模拟第一帧数据 (LCM) ---
    {
        // 1. 驱动层产生数据
        // msg 变量在栈上，指向堆内存。引用计数 = 1
        MsgPtr msg = driverReceiveData(TYPE_LCM_DOG_CTRL, "RUN"); 
        
        // 2. 扔给分发器
        // process 函数参数也是 MsgPtr，进入函数时引用计数变 2，出来变回 1
        dispatcher.dispatch(msg); 
        
        std::cout << "[主循环] 当前消息引用计数: " << msg.use_count() << std::endl;
        
    } // <--- msg 离开作用域。引用计数变 0。RawMessage 的析构函数在这里自动触发！

    // --- 模拟第二帧数据 (ROS) ---
    {
        MsgPtr msg2 = driverReceiveData(TYPE_ROS_LIDAR, "POINT_CLOUD_DATA_BLOB");
        dispatcher.dispatch(msg2);
    } // <--- msg2 离开作用域，自动析构

    // --- 模拟未知数据 ---
    {
        MsgPtr msg3 = driverReceiveData(0xFF, "UNKNOWN_CMD");
        dispatcher.dispatch(msg3);
    }

    std::cout << "=== 系统关闭 ===" << std::endl;
    return 0;
}