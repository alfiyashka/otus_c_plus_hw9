#ifndef __CONCURRENT_QUEUE__
#define __CONCURRENT_QUEUE__

#include <memory>
#include <mutex>
#include <utility>


template <typename T>
class ConcurrentQueue
{
private:
    struct Node
    {
        T data;
        std::unique_ptr<Node> next;
        Node(T data_): data(std::move(data_)) {}
    };

    std::unique_ptr<Node> head;
    Node* tail;
    std::mutex headMutex;
    std::mutex tailMutex;
    
public:
};

#endif