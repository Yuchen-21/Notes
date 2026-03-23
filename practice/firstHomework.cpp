// 第 1 周
// 单生产者单消费者队列
// 要求模版、条件变量、锁

#include<condition_variable>
#include<mutex>
#include<queue>
#include<thread>
#include<iostream>
// #include<typename T>
template<typename T>
class BlockingQueue {
public:
    BlockingQueue(std::size_t capacity) : capacity_(capacity) {}
    void push(T item){
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        //先把共享数据改好并释放锁，再通知等待线程, 所以要把通知放出来
        condition_variable_.notify_one();
    }
    T pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        condition_variable_.wait(lock, [this](){
            return !queue_.empty();
        });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }


private:
    std::mutex mutex_;
    std::condition_variable condition_variable_;
    std::queue<T> queue_;
    std::size_t capacity_;
};

int main(){
    BlockingQueue<int> queue(5);
    std::thread producer([&queue]() {
        for (int i = 1; i <= 5; ++i) {
            queue.push(i);
            std::cout << "push: " << i << std::endl;
        }
    });

    std::thread consumer([&queue]() {
        for (int i = 1; i <= 5; ++i) {
            int value = queue.pop();
            std::cout << "pop: " << value << std::endl;
        }
    });

    producer.join();
    consumer.join();

    return 0;
}