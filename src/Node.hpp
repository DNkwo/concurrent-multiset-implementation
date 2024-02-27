#ifndef NODE_HPP
#define NODE_HPP

#include <atomic>
#include <mutex>


//Node struct used for the Single-Lock and Optimistic Strategies
template <typename T>
struct Node {
    T data;
    int count;
    Node* next;
    std::mutex mtx; //every node has a mutex, (will be used for fine-grained implementations)

    Node(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};

//Node struct used for the Lock-Free strategy, some of the variables are wrapped in atomic wrappers for atomic operations
template <typename T>
struct Node_A {
    T data;
    std::atomic<int> count;
    std::atomic<Node_A<T>*> next;

    Node_A(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};


#endif