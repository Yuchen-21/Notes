# lamda表达式
## 概念
**随用随写，用完即扔**
它是一个匿名函数对象 (Closure / Functor)。 人话：它是一段可以在函数内部定义的、能“捕获”周围变量的逻辑代码块。
## 写法
```
[捕获列表](参数列表) mutable →返回值{函数体}
[](){ ... }
```
1. [] 捕获列表 (Capture Clause) —— 触手 🦑
这是 Lambda 最强大的地方。它可以伸出“触手”，把 Lambda 定义位置周围的变量抓进来用。
里面参数可以为
- []：光棍。我不抓任何外部变量，我只用我参数里的东西。

- [=]：复印机。把外面所有的局部变量都复制 (Copy) 一份进来。（只读，不能改，除非加 mutable）。

- [&]：提线木偶。把外面所有的局部变量的引用 (Reference) 抓进来。（我在里面改，外面的变量也会变）。

- [x, &y]：挑食。只复制 x，只引用 y，其他不看。

- [this]：抓自己。在类里面使用 Lambda 时，抓取当前对象的指针，以便访问类的成员变量。
2. () 参数列表 (Parameters) —— 嘴巴 👄

和普通函数一模一样。

    [](int a, int b) { return a + b; }

3. mutable —— 变身卡 🃏

    默认情况下，[=] 抓进来的变量是 const 的（只读）。

    如果你想在 Lambda 内部修改这些复制进来的变量，必须加上 mutable 关键字。

④ {} 函数体 —— 胃

逻辑处理的地方。
## 底层
后其实偷偷生成了一个类（类似下面这样）：
```cpp
int x = 10;
auto lam = [x](int a) { return x + a; };
------------------------
// 编译器自动生成的“匿名类”
class __Lambda_RandomName {
private:
    int x; // 对应 [x]，捕获的变量变成了成员变量
public:
    // 构造函数：负责把外部的 x 存进来
    __Lambda_RandomName(int _x) : x(_x) {}

    // 对应 () 和 {}
    // 注意：默认是 const 函数！这就是为什么默认不能修改 x 的原因
    int operator()(int a) const { 
        return x + a; 
    }
};
```
Lambda 不是魔法，它本质上是一个**自带状态的函数对象 (Functor)**。

## 应用
1. 场景一,配合sort排序
```cpp
std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
    if (a.level != b.level) {
        return a.level > b.level; // 等级高的在前面
    }
    return a.id < b.id; // 等级一样，ID 小的在前面
});
```
2. 场景二，智能指针的自定义删除器 🗑️
删除时不要 delete，而是放回池子
```cpp
std::shared_ptr<Bullet> b(new Bullet(), [this](Bullet* p) {
    // 这里的 this 是通过 Lambda 捕获进来的
    // 如果没有 Lambda，你很难把“回收逻辑”优雅地塞给 shared_ptr
    this->recycle(p); 
});
```
3. 场景三：回调函数与 UI 事件响应 🖱️
在写 GUI（比如 Qt）或者网络库时，经常有这种需求：“当按钮被点击时，更新一下这个 label 的文字”。
```cpp
int clickCount = 0;

// [&] 捕获了 clickCount 的引用
button.onClick([&]() {
    clickCount++;
    label.setText("Clicked: " + std::to_string(clickCount));
});
```

4. 场景四：多线程 (Concurrency) 🧵

你想开个线程后台处理数据，但不想专门写个线程函数。
```cpp
int data = 100;
std::thread t([data]() {
    // 这里是子线程运行的代码
    // data 是从主线程复制过来的（线程安全，因为是复制）
    std::cout << "Processing data: " << data << std::endl;
    // 这里的 data 修改不会影响主线程
});
t.join();
```

5. ⚠️ 高危预警：悬空引用 (Dangling Reference)
这是 Lambda 最容易导致 Core Dump (程序崩溃) 的地方。千万注意！
情景：你用 [&] 引用捕获了一个局部变量，然后把这个 Lambda 返回出去了，或者传给了一个异步线程。   
```cpp
std::function<void()> createTask() {
    int x = 10;
    // ❌ 危险！
    // 你捕获了 x 的引用 [&]，但函数结束时 x 就销毁了。
    return [&]() { 
        std::cout << x << std::endl; // 此时 x 已经是野指针了！
    };
}

void test() {
    auto task = createTask();
    task(); // 💥 崩溃！访问了一块已经释放的内存。
}
```
工程法则：

    如果 Lambda 是当场使用（比如传给 std::sort），用 [&] 没问题，最高效。

    如果 Lambda 会被存储起来或者延后执行（比如传给线程、定时器、或者作为返回值），必须用 [=] (值捕获) 或者 shared_ptr，绝对不能引用局部变量！
