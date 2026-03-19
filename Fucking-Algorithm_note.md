# Fucking-Algorithm_note
[TOC]
## 基础
### vetor
根据「数组」的特性，利用索引访问元素很高效，从尾部增删元素也是很高效的；而从中间或头部增删元素要涉及搬移数据，很低效。
#### 常用操作
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main() {
    int n = 10;
    // 数组大小为 10，元素值都为 0
    vector<int> nums(n);
    // 输出 0 (false)
    cout << nums.empty() << endl;
    // 输出：10
    cout << nums.size() << endl;

    // 在数组尾部插入一个元素 20
    nums.push_back(20);
    // 输出：11
    cout << nums.size() << endl;

    // 得到数组最后一个元素的引用
    // 输出：20
    cout << nums.back() << endl;

    // 删除数组的最后一个元素（无返回值）
    nums.pop_back();
    // 输出：10
    cout << nums.size() << endl;

    // 可以通过方括号直接取值或修改
    nums[0] = 11;
    // 输出：11
    cout << nums[0] << endl;

    // 在索引 3 处插入一个元素 99
    nums.insert(nums.begin() + 3, 99);

    // 删除索引 2 处的元素
    nums.erase(nums.begin() + 2);

    // 交换 nums[0] 和 nums[1]
    swap(nums[0], nums[1]);

    // 遍历数组
    // 0 11 99 0 0 0 0 0 0 0
    for (int i = 0; i < nums.size(); i++) {
        cout << nums[i] << " ";
    }
    cout << endl;
}
```
### list
双链表
一般来说，当我们想在头部增删元素时会使用双链表，因为它在头部增删元素的效率比 vector 高。但我们通过索引访问元素，这种场景下我们会使用 vector。
#### 常用操作
```cpp
#include <list>
int n = 7;
// 初始化一个空的双向链表 lst
std::list<int> lst;
// 初始化一个大小为 n 的链表 lst，链表中的值默认都为 0
std::list<int> lst(n);
// 初始化一个包含元素 1, 3, 5 的链表 lst
std::list<int> lst{1, 3, 5};
// 初始化一个大小为 n 的链表 lst，其中值都为 2
std::list<int> lst(n, 2);
```
```cpp
#include <iostream>
#include <list>
using namespace std;

int main() {
    // 初始化链表
    list<int> lst{1, 2, 3, 4, 5};
    // 检查链表是否为空，输出：false
    cout << lst.empty() << endl;
    // 获取链表的大小，输出：5
    cout << lst.size() << endl;
    // 在链表头部插入元素 0
    lst.push_front(0);
    // 在链表尾部插入元素 6
    lst.push_back(6);
    // 获取链表头部和尾部元素，输出：0 6
    cout << lst.front() << " " << lst.back() << endl;
    // 删除链表头部元素
    lst.pop_front();
    // 删除链表尾部元素
    lst.pop_back();
    // 在链表中插入元素
    auto it = lst.begin();
    // 移动到第三个位置
    advance(it, 2);
    // 在第三个位置插入 99
    lst.insert(it, 99);
    // 删除链表中某个元素
    it = lst.begin();
    // 移动到第二个位置
    advance(it, 1);
    // 删除第二个位置的元素
    lst.erase(it);
    // 遍历链表
    // 输出：1 99 3 4 5
    for (int val : lst) {
        cout << val << " ";
    }
    cout << endl;
    return 0;
}
```
### queue 队列
队列适用于只允许从一端（队尾）添加元素、从另一端（队头）移除元素的场景。FIFO。
```cpp
#include <iostream>
#include <queue>
using namespace std;

int main() {
    // 初始化一个空的整型队列 q
    queue<int> q;

    // 在队尾添加元素
    q.push(10);
    q.push(20);
    q.push(30);

    // 检查队列是否为空，输出：false
    cout << q.empty() << endl;

    // 获取队列的大小，输出：3
    cout << q.size() << endl;

    // 获取队列的队头和队尾元素，输出：10 和 30
    cout << q.front() << " " << q.back() << endl;

    // 删除队头元素
    q.pop();

    // 输出新的队头元素：20
    cout << q.front() << endl;

    return 0;
}
```
### stack 栈
栈是一种后进先出（LIFO）的数据结构，栈适用于只允许在一端（栈顶）添加或移除元素的场景。
```cpp
#include <iostream>
#include <stack>
using namespace std;

int main() {

    // 初始化一个空的整型栈 s
    stack<int> s;

    // 向栈顶添加元素
    s.push(10);
    s.push(20);
    s.push(30);

    // 检查栈是否为空，输出：false
    cout << s.empty() << endl;

    // 获取栈的大小，输出：3
    cout << s.size() << endl;

    // 获取栈顶元素，输出：30
    cout << s.top() << endl;

    // 删除栈顶元素
    s.pop();

    // 输出新的栈顶元素：20
    cout << s.top() << endl;

    return 0;
}
```
### unordered_map 哈希表
unordered_map 是 C++ 标准库中的一种哈希表实现，它提供了基于键值对（key-value）的存储，提供了常数时间复杂度的查找、插入和删除键值对的操作。
```cpp
#include <unordered_map>
using namespace std;

// 初始化一个空的哈希表 map
unordered_map<int, string> hashmap;

// 初始化一个包含一些键值对的哈希表 map
unordered_map<int, string> hashmap{{1, "one"}, {2, "two"}, {3, "three"}};
```
#### 特别注意：访问不存在的键会自动插入键值对

在 C++ 的哈希表中，如果你访问一个不存在的键，它会自动创建这个键，对应的值是默认构造的值。
这一点和其他语言不同，需要格外注意。记住访问值之前要先判断键是否存在，否则可能会意外地创建新键，导致算法出错。详见下面的示例。
```cpp
#include <iostream>
#include <unordered_map>
using namespace std;

int main() {
    // 初始化哈希表
    unordered_map<int, string> hashmap{{1, "one"}, {2, "two"}, {3, "three"}};

    // 检查哈希表是否为空，输出：0 (false)
    cout << hashmap.empty() << endl;

    // 获取哈希表的大小，输出：3
    cout << hashmap.size() << endl;

    // 查找指定键是否存在
    // 注意 contains 方法是 C++20 新增的
    // 输出：Key 2 -> two
    if (hashmap.contains(2)) {
        cout << "Key 2 -> " << hashmap[2] << endl;
    } else {
        cout << "Key 2 not found." << endl;
    }

    // 获取指定键对应的值，若不存在会返回默认构造的值
    // 输出空字符串
    cout << hashmap[4] << endl;

    // 插入一个新的键值对
    hashmap[4] = "four";

    // 获取新插入的值，输出：four
    cout << hashmap[4] << endl;

    // 删除键值对
    hashmap.erase(3);

    // 检查删除后键 3 是否存在
    // 输出：Key 3 not found.
    if (hashmap.contains(3)) {
        cout << "Key 3 -> " << hashmap[3] << endl;
    } else {
        cout << "Key 3 not found." << endl;
    }

    // 遍历哈希表
    // 输出（顺序可能不同）：
    // 4 -> four
    // 2 -> two
    // 1 -> one
    for (const auto &pair: hashmap) {
        cout << pair.first << " -> " << pair.second << endl;
    }

    // 特别注意，访问不存在的键会自动创建这个键
    unordered_map<int, string> hashmap2;

    // 键值对的数量是 0
    cout << hashmap2.size() << endl; // 0

    // 访问不存在的键，会自动创建这个键，对应的值是默认构造的值
    cout << hashmap2[1] << endl; // empty string
    cout << hashmap2[2] << endl; // empty string

    // 现在键值对的数量是 2
    cout << hashmap2.size() << endl; // 2

    return 0;
}
```
### 哈希集合 unordered_set  
unordered_set 是 C++ 标准库中的一种哈希集合实现，用于存储不重复的元素，常见使用场景是对元素进行**去重**。
```cpp
#include <unordered_set>
using namespace std;

// 初始化一个空的哈希集合 set
unordered_set<int> uset;

// 初始化一个包含一些元素的哈希集合 set
unordered_set<int> uset{1, 2, 3, 4};
```
常用操作
```cpp
#include <iostream>
#include <unordered_set>
using namespace std;

int main() {
    // 初始化哈希集合
    unordered_set<int> hashset{1, 2, 3, 4};

    // 检查哈希集合是否为空，输出：0 (false)
    cout << hashset.empty() << endl;

    // 获取哈希集合的大小，输出：4
    cout << hashset.size() << endl;

    // 查找指定元素是否存在
    // 输出：Element 3 found.
    if (hashset.contains(3)) {
        cout << "Element 3 found." << endl;
    } else {
        cout << "Element 3 not found." << endl;
    }

    // 插入一个新的元素
    hashset.insert(5);

    // 删除一个元素
    hashset.erase(2);
    // 输出：Element 2 not found.
    if (hashset.contains(2)) {
        cout << "Element 2 found." << endl;
    } else {
        cout << "Element 2 not found." << endl;
    }

    // 遍历哈希集合
    // 输出（顺序可能不同）：
    // 1
    // 3
    // 4
    // 5
    for (const auto &element : hashset) {
        cout << element << endl;
    }

    return 0;
}
```
## 做算法题时的选择, 传值 vs 传引用

以我的经验，如果是传递基本类型，比如 int、bool 等，用传值比较多，因为这类数据一般不需要在函数内部修改，而且复制的开销很小。

如果是传递容器数据结构，比如 vector、unordered_map 等，用传引用比较多，因为可以避免复制数据副本的开销，而且容器一般需要在函数内部修改。

特别注意一个可能出现的问题，就是当递归函数的参数中有容器数据结构时，千万别使用传值的方式，否则每次递归都会创建一个数据副本，消耗大量的内存和时间，非常容易导致超时或者超内存的错误。

## 刷题tips
提交时记得删除打印语句

因为打印语句输入 IO 操作，会影响代码的执行效率，所以提交前一定要记得删除，否则会导致算法的效率大幅下降。
## 划重点，看清题目要求

看题要仔细，把题目给的所有信息都看清楚。很多读者只看题干和示例，却忽略了题目下方给出的数据规模等补充信息。

比如这道题，下面补充说明了 nums 的长度范围，和其中元素的大小范围，还有一个很重要的点，就是告诉你 nums 中并没有重复元素。

这是个关键信息，因为如果有多个相同的重复元素，那么你到底应该返回哪个索引呢？这些信息一般都会在补充说明中给出，一定要看清楚。
### 两数之和
```cpp
#include <unordered_set>
#include <iostream>
#include <vector>
std::vector <int> TwoSum (){

}
int main() {
    unordered_set<int> hashset nums;
    
}
```

### 机器人工程师的 C++ STL “生存急救包”
A. std::vector (动态数组) - 最常用

    增：push_back(val) (尾部插入)

    删：pop_back() (尾部删除)

    查：vec[i] (下标访问), back() (最后一个)

    属性：size() (大小), empty() (是否为空)

    排序：sort(vec.begin(), vec.end())

B. std::unordered_map / std::unordered_set (哈希表) - 查找神器

    增：map[key] = val (Map专用), set.insert(val)

    查 (通用)：if (map.find(key) != map.end()) (判断是否存在)

    查 (偷懒版)：if (map.count(key)) (因为 key 唯一，所以存在返回 1，不存在返回 0，等同于 bool)

    取值：map[key]

C. std::stack (栈) - 括号匹配、DFS

    入：push(val)

    出：pop() (注意：不返回元素)

    看：top() (返回栈顶元素的引用)

D. std::queue (队列) - BFS、任务调度

    入：push(val)

    出：pop()

    看：front() (队头), back() (队尾)