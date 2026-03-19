#include<vector>
template <typename T>
class ObjectPool{
private:
    object;
    std::vector<std::unique_ptr<T>> free_objects;
public:
    ObjectPool(){
        
    }
    ~ObjectPool(){
        
    }
};

ObjectPool<Bullet> bullet;
ObjectPool<Monster> Monster;

