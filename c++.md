# c++ 学习笔记
[TOC]
## C++ 是什么
***C++ 是一种静态类型的、编译式的、通用的、大小写敏感的、不规则的编程语言，支持过程化编程、面向对象编程和泛型编程。***
**面向对象程序设计**
C++ 完全支持面向对象的程序设计，包括面向对象开发的四大特性：
- **封装（Encapsulation）**：封装是将数据和方法组合在一起，对外部隐藏实现细节，只公开对外提供的接口。这样可以提高安全性、可靠性和灵活性。
- **继承（Inheritance）**：继承是从已有类中派生出新类，新类具有已有类的属性和方法，并且可以扩展或修改这些属性和方法。这样可以提高代码的复用性和可扩展性。
- **多态（Polymorphism）**：多态是指同一种操作作用于不同的对象，可以有不同的解释和实现。它可以通过接口或继承实现，可以提高代码的灵活性和可读性。
- **抽象（Abstraction）**：抽象是从具体的实例中提取共同的特征，形成抽象类或接口，以便于代码的复用和扩展。抽象类和接口可以让程序员专注于高层次的设计和业务逻辑，而不必关注底层的实现细节。
## c++概念
### Lvalue 左值
指向特定内存具有名称的值，即具名对象，有稳定的可识别的内存地址
函数形参也是左值
字符串字面量是特例，为左值，本质上是常量字符数组
左值 (Lvalue)：有名字、有内存地址、能持久存在的对象。比如 int a 中的 a。
### Rvalue 右值
指的是临时短暂的表达式或值，没有稳定的可识别的内存地址，临时对象
右值 (Rvalue)：没名字（或者即将销毁）、通常是临时对象。比如 func() 的返回值、字面量 10、或者表达式 a + b 的结果。
### 左值引用
例题：
```cpp
void fuck(int&);
void fuck(int&&);
int &&a{1};
fuck(a);
```
凡是“有名字、能取地址”的变量表达式，都是左值
### &&到底是什么
1.逻辑里的与 
特性：短路求值（Short-circuit evaluation）。如果前一个为假，后一个不会执行。
```cpp
if (ptr != nullptr && ptr->isValid()) {
    // ...
}
```
2.右值引用
- C++11 引入的特性。
- 目的：合法地通过偷窃来避免拷贝
```cpp
// 旧板C++ 98
void processImage(const std::vector<uint8_t>& img) {
    // 这里 img 是原来的引用，只读
    std::vector<uint8_t> my_copy = img; // 必须深拷贝 5MB 数据，慢！
    // 修改 my_copy...
}
// 现代写法 (C++11) - 移动语义 (Move Semantics)： 如果你确定传入的图像之后不再用了（比如是临时生成的，或者是上一层传下来的），你可以用 && 接住它。
// 注意参数里的 &&
void processImage(std::vector<uint8_t>&& img) {
    // img 绑定到了一个“临时对象”或者“被明确放弃所有权的对象”
    
    // 这一步发生“移动构造”：
    // my_data 只需要把 img 内部的指针拿过来，
    // 然后把 img 内部指针置空。
    // 耗时：几乎为 0（只是几个指针赋值），没有 5MB 的 memcpy。
    std::vector<uint8_t> my_data = std::move(img); 
}
```
“调用者，请把 msg 的所有权交给我。我会把它的内部资源（内存指针）‘偷’走直接发出去。你调用完这个函数后，你手里的 msg 就变空了，不要再用它了。”
这就是现代中间件消除不必要的内存拷贝的核心手段。
### 万能引用 (Universal Reference) —— 进阶陷阱
如果 T 是一个模板参数，或者是 auto，那么 T&& 不一定是右值引用，它叫万能引用（Forwarding Reference）。
```cpp
template<typename T>
void wrapper(T&& arg) {
    // 这里 arg 既能接左值，也能接右值
    // 它是为了“完美转发”（Perfect Forwarding）存在的
    some_other_function(std::forward<T>(arg));
}
```
如果你传左值进来，T&& 就折叠成左值引用。
    如果你传右值进来，T&& 就是右值引用。
应用场景： 你在 aimrt 里看到的类似 RegisterCallback 或者 CreateTask 这种泛型接口，通常都用了万能引用，目的是把参数原封不动（保留左值/右值属性）地传给下一层。
### 移动语义
can not move: lvalue （lvalue和xvalue被认为是generalized lvalue）
    can move：xvalue pvalue  （这行都是rvalue）
pvalue：纯右值，比如函数返回值或临时对象
xvalue：将亡值，即将要销毁的对象，又左值使用move后得到
不能直接应用移动语义到 lvalue 上，因为 lvalue 是持久存在的对象，移动后它的状态会改变（变成空），这不是我们期望的。而可以移动的是右值。
左值引用和右值引用都是左值。
### 顺时针螺旋移动法
总结一句话：“右边是数组/函数，左边是指针/类型，先右后左，遇墙调头。”
```cpp
void (*fp)(int);
```
Start: fp 是变量名。
Right: 遇到 )，被挡住，向左看。
Left: 遇到 * → fp 是一个指针。
Out: 跳出括号 (*fp)，现在看 *fp 的右边。
Right: 遇到 (int) → 该指针指向一个函数（参数是 int）。
Left: 遇到 void → 该函数返回 void。
结论：fp 是一个指向函数的指针，该函数接受 int 参数并返回 void。

```cpp
void (*signal(int, void (*)(int)))(int);
```
这行代码如果不按螺旋法读，非常容易晕。让我们画圈：
Start: signal 是变量名。
Right: 遇到 (int, void (*)(int)) → signal 是一个函数。
    它有两个参数：一个是 int，另一个是 void (*)(int)（即：指向接受int返回void的函数的指针）。
Left: 遇到 * → signal 函数返回一个指针。
Right: 跳出第一层括号，看右边 (int) → 返回的指针指向一个函数（参数是 int）。
Left: 遇到 void → 这个指向的函数返回 void。
结论：signal 是一个函数，它接受一个整型和一个函数指针作为参数，并返回一个函数指针（该指针指向一个接受整型参数并返回 void 的函数）
- 如果用using
```cpp
// 现代 C++ 写法
using SignalHandler = void (*)(int);
SignalHandler signal(int signum, SignalHandler handler);
```
### 常见梳理
```cpp
const int* p;        // 指向常量的指针
int* const p;        // 常量指针
int (*fp)(int);      // 函数指针
int* fp(int);        // 指针函数
int* arr[10];        // 指针数组
int (*arr)[10];      // 数组指针
void (A::*fp)(int);  // 成员函数指针
```
### cpp内存布局
从  |    栈 (Stack) —— "自动管理区"
高  |    内存映射区 (Memory Mapping Segment)
到  |    堆 (Heap) —— "程序员的责任田"
低  |    未初始化数据段 (BSS Segment)
    |    初始化数据段 (Initialized Data Segment / .data)    
    ↓    代码段 (Text Segment / Code Segment)
**1. 栈 (Stack) —— "自动管理区"**
位置：位于用户空间的最高处，向低地址增长（与堆相对生长）。
内容：
- **局部变量（Local Variables）**。
- **函数参数**。
- 返回地址（函数调用结束后跳回哪里）。
- 栈帧（Stack Frame）信息。
特性：
- 极快：分配只是移动栈指针（SP寄存器），几乎零开销。
- 自动管理：离开作用域（Scope）自动释放（RAII 的基础）。
- 空间有限：Linux 默认通常是 8MB（可通过 ulimit -s 查看）。递归过深或局部数组过大会导致 Stack Overflow。
- **LIFO：后进先出**。
**2. 内存映射区 (Memory Mapping Segment)**
位置：通常位于堆和栈之间。
内容：
- 动态链接库（.so 文件）加载的地方。
- mmap 系统调用分配的内存（用于文件映射或大块内存分配）。
- 共享内存（Shared Memory）：ROS2 / 自动驾驶中常用的 IPC 通信方式，通常就在这里。
**3. 堆 (Heap) —— "程序员的责任田"**
位置：位于 BSS 段之上，向高地址增长。
内容：动态分配的内存。
关键字：**new, delete, malloc, free**。
特性：
- 手动管理：由程序员控制生命周期。如果忘了释放，就是内存泄漏。
- 碎片化：频繁分配释放会导致内存碎片。
- 慢：分配涉及系统调用和复杂的内存管理算法。
中间件开发注意：在自动驾驶或机器人控制循环（Control Loop）中，尽量避免在热路径（Hot Path）上进行 Heap 分配，因为 new 的耗时是不确定的，会引起抖动（Jitter）。
**4. 未初始化数据段 (BSS Segment)**
全称：Block Started by Symbol（由于历史原因的名字）。
内容：存放未初始化（或初始化为 0）的全局变量和静态变量。
示例：
```cpp
int g_array[1024];         // 全局变量，未初始化 -> BSS
static int s_flag;         // 静态变量，默认初始化为0 -> BSS
```
特性：
- 不占磁盘空间：在可执行文件中，BSS 段只记录数据的大小，不记录具体内容（因为都是 0）。
- 运行时清零：程序加载时，操作系统会将这块内存全部置零。
**5. 代码段 (Text Segment / Code Segment)**
位置：最低地址区域。
内容：存放编译后的机器指令（Machine Code）。
特性：
- 只读（Read-Only）：防止程序意外修改指令导致崩溃。
- 共享（Shared）：如果同时运行多个相同的程序实例（如多个节点进程），这部分内存可以被共享，节省资源。
- 通常还包含只读常量数据（.rodata），例如字符串字面量 "Hello World"。

### RAII（资源获取即初始化）编程范式思想
它确保资源如动态内存、文件句柄和锁在构造函数中获取，并在析构函数中释放，从而避免资源泄漏，提高代码的异常安全性和维护性。
- 构造函数：在对象创建时自动调用，用于资源的获取。
- 析构函数：在对象销毁时自动调用，用于资源的释放。

### 类的继承
当创建一个类时，您不需要重新编写新的数据成员和成员函数，只需指定新建的类继承了一个已有的类的成员即可。这个已有的类称为基类，新建的类称为派生类。
继承代表了 **is a** 关系。例如，哺乳动物是动物，狗是哺乳动物，因此，狗是动物，等等。

### 多态
C++ 多态允许使用基类指针或引用来调用子类的重写方法，从而使得同一接口可以表现不同的行为。
多态使得代码更加灵活和通用，程序可以通过基类指针或引用来操作不同类型的对象，而不需要显式区分对象类型。这样可以使代码更具扩展性，在增加新的形状类时不需要修改主程序。
#### 使用多态的优势：
代码复用：通过基类指针或引用，可以操作不同类型的派生类对象，实现代码的复用。
扩展性：新增派生类时，不需要修改依赖于基类的代码，只需要确保新类正确重写了虚函数。
解耦：多态允许程序设计更加模块化，降低类之间的耦合度。
只有通过基类的指针或引用调用虚函数时，才会发生多态

### 数据结构
std::vector (动态数组)：

    像一排紧挨着的剧院座位。

    所有数据在内存里是连续存放的。

    优势：你想找第 100 个人，直接算一下位置就能瞬间跳过去（随机访问快）。

    劣势：如果你想在第 5 个位置插进一个人，后面所有人都得起身往后挪一个位置（插入/删除慢）。
常用成员函数
以下是 <vector> 中的一些常用成员函数：
**函数	说明
push_back(const T& val)	在末尾添加元素
pop_back()	删除末尾元素
at(size_t pos)	返回指定位置的元素，带边界检查
operator[]	返回指定位置的元素，不带边界检查
front()	返回第一个元素
back()	返回最后一个元素
data()	返回指向底层数组的指针
size()	返回当前元素数量
capacity()	返回当前分配的容量
reserve(size_t n)	预留至少 n 个元素的存储空间
resize(size_t n)	将元素数量调整为 n
clear()	清空所有元素
insert(iterator pos, val)	在指定位置插入元素
erase(iterator pos)	删除指定位置的元素
begin() / end()	返回起始/结束迭代器**

1. pop_back() 的三大铁律

    只管杀，不管埋：它会调用最后一个元素的析构函数（如果要析构的话），然后把 vector 的 size 减 1。

    它是哑巴（返回 void）：这是新手最容易踩的坑！

        错误写法：auto data = vec.pop_back(); ❌

        原因：pop_back() 没有任何返回值。它默默地把东西删了，不会把删掉的东西给你。

    不检查边界：如果 vector 已经是空的，你再调用 pop_back()，程序会未定义行为（通常是崩溃）
**删除容器的最后一个元素**

std::list (双向链表)：

    像寻宝游戏。

    数据在内存里是分散的，每个人手里拿着一张纸条，写着下一个人在哪。

    优势：你想在任何地方加一个人，只需要改一下前后的纸条指向，不需要任何人移动（插入/删除快）。

    劣势：你想找第 100 个人，必须从第 1 个开始顺着纸条一个个找（随机访问慢）。
std::map (红黑树 Red-Black Tree)：

    像一本字典。

    内部始终保持自动排序（按 Key 从小到大）。

    查找速度：稳定，像二分查找一样，每次排除一半。时间复杂度是 O(logN)。

    优势：如果你需要遍历数据（比如“列出所有在这个范围内的用户”），它是首选。

std::unordered_map (哈希表 Hash Table)：

    像一堆分类的储物柜。

    内部是乱序的。它通过一个数学公式（哈希函数）直接算出数据应该放在哪个柜子里。

    查找速度：极快，理论上是一步到位。平均时间复杂度是 O(1)。

    优势：纯粹为了快！只要知道 Key，瞬间拿到 Value。
std::stack (栈)：

    规则：后进先出 (LIFO - Last In, First Out)。

    比喻：像是在洗盘子，你只能把新盘子叠在最上面，也只能从最上面取走盘子。你碰不到底下的盘子。

std::queue (队列)：

    规则：先进先出 (FIFO - First In, First Out)。

    比喻：就像排队买奶茶。先排队的人先买到，不能插队，也不能从中间离开。
迭代器（Iterator）是一种设计模式，用于遍历容器（如数组、列表、树等）中的元素，而无需暴露容器的内部实现细节。
虽然大家都有 begin() 和 end()，但是不同容器提供的迭代器，能力是不一样的。这就好比：

std::vector 的迭代器：瞬移法师 (Random Access Iterator)

    它可以做 it + 5！

    它可以瞬间跳到第 5 个元素，不需要经过前 4 个。

    能力：最强。支持 ++, --, +n, -n, []。

std::list 的迭代器：步行路人 (Bidirectional Iterator)

    它不能做 it + 5！如果你写了，编译器直接报错。

    因为它不知道第 5 个在哪里，它必须老老实实地执行 5 次 ++it 才能走过去。

    能力：中等。支持 ++, --（可以回头），但不支持跳跃。

std::forward_list (单向链表) 的迭代器：过河卒子 (Forward Iterator)

    它甚至不能做 --it（回头）。

    只能永远向前走。

    能力：最弱。
**迭代器是连接 容器 和 算法 的桥梁。**  
所有的迭代器都能用 *it (取值) 和 ++it (前进);
只有 连续内存 (vector/array) 的迭代器才能 随机跳跃 (it + n) 
### 算法
几乎所有的 STL 算法都长这个样子，它们甚至不关心你用的是 vector 还是 list，它们只认迭代器：
**std::算法名(起点,终点,可选规则)**
只要你喂给它 begin() 和 end()，它就会自动处理中间的所有元素。
常用的3个写法：
#### Find
场景：在 vector 里找有没有数字 5。
```cpp
auto it = std::find(vec.begin(), vec.end(), 5);
// 判断是否找到（如果没找到，它会一直走到 end() 也就是终点线外）
if (it != vec.end()) {
    std::cout << "找到了！";
}
```
#### Sort

```cpp
// 默认从小到大
std::sort(vec.begin(), vec.end());
// 同时还支持重载，标准库中还有从大到小排序，或者自定义排序规则。   
std::sort(vec.begin(), vec.end(), std::greater<int>()); 

```
#### Count If
Lambda 表达式 (匿名函数)。 别被名字吓到，它就是一个**“一次性的小函数”**，写法是 []{...}。
```cpp
// std::count_if(起点, 终点, 筛选规则);
int n = std::count_if(songs.begin(), songs.end(), [](const Song& s) {
    return s.duration > 180; // 把规则直接写在这里
});
```
优势：代码读起来像英语句子一样自然 —— "Count if song duration is greater than 180".

#### 小结
- 线性序列：vector 是连续内存王者，list 是拼接特种兵。
- 关联容器：map 负责有序，unordered_map 负责极速查找。
- 迭代器 & 算法：它们是操作容器的通用接口。
- 架构设计：如何组合不同的容器来取长补短（Map + Unordered_map）

### 智能指针

在写现代 C++ 代码时，请遵循这个决策树：

    默认情况 → 用 std::unique_ptr。

        效率极高（和裸指针几乎一样），零内存开销，最安全。

    真的需要多个人同时拥有同一个对象吗？ → 用 std::shared_ptr。

        注意：它比 unique_ptr 慢一点点（因为要维护计数器）。

    如果发生循环引用 → 把其中一个改成 std::weak_ptr。

    绝对不要 → 使用 auto_ptr (这是上古时代的弃用产物，千万别碰)。

### 引用与指针
引用 (Reference &) —— 绰号

    概念：它是变量的别名。

    比喻：你的名字叫“李明”，我给你起了个绰号叫“小明”。“小明”就是“李明”。

        打“小明”一拳，“李明”会疼。

        特性：

            从一而终：你一旦叫“小明”，这就绑定在你身上了，不能半路改去指代隔壁老王。

            不能为空：世界上不可能存在一个“不指代任何人”的绰号。你必须先有人，才能起绰号。

指针 (Pointer *) —— 门牌号

    概念：它是一个存放内存地址的变量。

    比喻：这是一张纸条，上面写着你家的地址（比如“南京路 88 号”）。

        特性：

            见异思迁：我可以擦掉纸条上的地址，改写成隔壁老王的地址。

            可以为空：纸条上可以什么都不写（nullptr），表示“不知道要去哪”。
### 函数传参
档位 A：传值 (Pass by Value) -> void func(int x)

    发生什么：克隆。把数据复制一份给函数。

    适用：int, bool, char 这种极小的基础类型。

    缺点：如果传一个 std::vector 或大对象，复制会极其耗时！

档位 B：传 Const 引用 (Pass by Const Reference) -> void func(const string& s)

    发生什么：只读别名。给函数一个“绰号”，并且规定“只能看不能改”。

    适用：90% 的场景。只要是大对象（vector, string, 自定义类），且函数内部不需要修改它，必须用这个。

    优点：既没有复制的开销（极快），又保证了数据安全（只读）。

档位 C：传指针 (Pass by Pointer) -> void func(int* p)

    发生什么：给地址。

    适用：

        可选参数：如果你需要传入“空”（表示忽略这个参数），只能用指针（因为引用不能为空）。

        C 语言接口兼容：调用老的 C 库时。

# 题目：一个简单的“通用对象池 (Object Pool)”

    背景：在游戏里，子弹射击非常频繁。如果每发一颗子弹都 new，撞墙了都 delete，CPU 会累死。

    解决：我们要预先创建好一堆子弹放在池子里。用的时候拿出来，不用了放回去（而不是销毁）。

    挑战点（融合你刚学的知识）：

        模板：这个池子应该既能存 Bullet，也能存 Monster。所以类要是 template <typename T> class ObjectPool。

        容器：用 std::vector<std::unique_ptr<T>> 来存放这些闲置的对象。

        RAII：能否设计一个机制，当借出去的对象“死”了的时候，自动弹回池子里，而不是被 delete？
解：
难点在于：怎么把借出去的对象“自动收回来”，而不是让它被销毁；
这里我们要用到你刚学的两个大招：

    Lambda 表达式（作为自定义删除器）。

    std::shared_ptr（利用它的引用计数和灵活性）
1. 核心设计图解

    仓库 (Pool)：存着一堆 std::unique_ptr<T>（没人用的）。

    借出 (Acquire)：

        用户来借，我们从仓库弹出一个。

        关键点：我们不直接给用户裸指针，也不给普通的 unique_ptr。

        我们给用户一个 “特制”的 std::shared_ptr。

    归还 (Return)：

        在这个“特制指针”的删除器 (Deleter) 里，我们写一段代码：“不要 delete 我！把我塞回仓库去！”
```cpp
#include <iostream>
#include <vector>
#include <memory> // 必须包含，为了 smart pointers
#include <functional> // 为了 std::function

// 假设这是你的子弹类
class Bullet {
public:
    Bullet() { std::cout << "  [创建] 一颗新子弹" << std::endl; }
    ~Bullet() { std::cout << "  [销毁] 子弹彻底报废" << std::endl; }
    void fire() { std::cout << "  biu! biu! biu!" << std::endl; }
};

template <typename T>
class ObjectPool {
private:
    // 1. 闲置仓库：没人用的时候，所有权归池子，用 unique_ptr 存好
    std::vector<std::unique_ptr<T>> pool_;

public:
    ObjectPool() {}
    
    // 析构时，vector 会自动销毁里面所有的 unique_ptr，
    // 所以真正的内存释放是在这里发生的 (RAII)
    ~ObjectPool() {
        std::cout << "池子关闭，清空所有库存..." << std::endl;
    }

    // 2. 借出函数 (核心魔法在这里)
    std::shared_ptr<T> acquire() {
        std::unique_ptr<T> ptr;

        // A. 看看仓库里有没有存货
        if (!pool_.empty()) {
            // 有存货：拿出来 (move 出来，pool_ 里那个就空了)
            ptr = std::move(pool_.back());
            pool_.pop_back();
            std::cout << "-> 从池子里复用旧对象" << std::endl;
        } else {
            // 没存货：只能造个新的
            ptr = std::make_unique<T>();
            std::cout << "-> 池子空了，新建对象" << std::endl;
        }

        // B. 制作“回旋镖”指针
        // 我们把 unique_ptr 转换成 shared_ptr 给用户。
        // 并在第二个参数里写一个 Lambda：当这个 shared_ptr 没人用时，执行这段代码
        std::shared_ptr<T> sharedPtr(
            ptr.release(), // 1. 拿出裸指针给 shared_ptr 管理
            [this](T* p) { // 2. 自定义删除器 (捕获 this 指针，为了能操作 pool_)
                // ⚠️ 重点：这里不 delete p，而是把它变回 unique_ptr 塞回 vector！
                this->pool_.push_back(std::unique_ptr<T>(p));
                std::cout << "<- 对象使用完毕，自动归还池子" << std::endl;
            }
        );

        return sharedPtr;
    }
    
    // 查看库存数量
    size_t size() const { return pool_.size(); }
};

int main() {
    // 创建一个子弹池
    ObjectPool<Bullet> bulletPool;

    std::cout << "--- 第一轮射击 ---" << std::endl;
    {
        // 借出一颗子弹
        auto b1 = bulletPool.acquire(); 
        b1->fire();
        
        // 借出第二颗子弹
        auto b2 = bulletPool.acquire();
        b2->fire();
        
        // 此时 b1, b2 还在作用域内，没人还
        std::cout << "当前池子库存: " << bulletPool.size() << std::endl;
        
    } // <--- 出了这个大括号，b1 和 b2 析构。
      // 神奇的事情发生了：它们不会被 delete，而是触发了 Lambda，飞回了池子！

    std::cout << "\n--- 第二轮射击 ---" << std::endl;
    {
        // 再次借出！这次应该复用之前的，而不是创建新的
        auto b3 = bulletPool.acquire(); 
        b3->fire();
        
        std::cout << "当前池子库存: " << bulletPool.size() << std::endl;
    }

    std::cout << "\n--- 程序结束 ---" << std::endl;
    return 0;
}
```
### 函数对象 Functor
Functor (函数对象)： 就是一个砖头（对象），我们给它装了一个把手（operator()），让它看起来像个锤子。

    因为它本质是对象，所以它有脑子（可以存成员变量，记住状态）。

    因为它有 operator()，所以它可以像函数一样被调用。
要让一个类变成 Functor，只需要做一件事：重载 () 运算符。
```cpp
class SayHello {
public:
    // 这就是那个“把手”
    void operator()() {
        std::cout << "Hello, Functor!" << std::endl;
    }
};

void test() {
    SayHello obj; // 1. 创建一个对象 (是个砖头)
    
    obj();        // 2. 像调用函数一样调用它！(因为有把手)
    // 输出: Hello, Functor!
}
```
关键在于 “状态 (State)”。
```cpp
class IsGreaterThan {
private:
    int threshold; // 🧠 它的“大脑”，用来存阈值

public:
    // 🏗️ 构造函数：初始化时设定阈值
    IsGreaterThan(int t) : threshold(t) {}

    // 🔨 调用操作符：真正干活的地方
    bool operator()(int n) const {
        return n > threshold; // 用大脑里记的数据和 n 比较
    }
};

void usage() {
    std::vector<int> nums = {1, 6, 3, 10, 8};

    // 场景 A: 我想删掉大于 5 的
    // IsGreaterThan(5) 创建了一个记住了 "5" 的对象
    std::remove_if(nums.begin(), nums.end(), IsGreaterThan(5));

    // 场景 B: 用户输入多少我就删多少
    int userInput;
    std::cin >> userInput;
    // IsGreaterThan(userInput) 创建了一个记住了 "userInput" 的对象
    std::remove_if(nums.begin(), nums.end(), IsGreaterThan(userInput)); 
}
```
不仅仅是传递了逻辑（大于），还传递了数据（阈值）。
#### functor 保持状态
普通函数调用完，栈内存就释放了，什么都留不下。
 Functor 是个对象，只要对象不死，它的成员变量就一直还在。它可以累积数据。
 Functor (函数对象) = 类 + operator()。

    本质：它是一个有属性、有状态的函数。

    使用场景：

        需要传入参数来定制函数行为时（比如上面的 IsGreaterThan）。

        需要函数调用之间保持状态时（比如计数器）。

        配合 STL 算法（sort, find_if, for_each）。

一句话心法：如果你觉得一个普通的函数不够用，因为它记不住东西，那就把它封装成一个 Functor！