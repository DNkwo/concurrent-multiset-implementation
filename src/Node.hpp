#ifndef NODE_HPP
#define NODE_HPP

#include <atomic>
#include <mutex>

template <typename T>
struct Node {
    T data;
    int count;
    Node* next;
    std::mutex mtx;

    Node(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};

template <typename T>
struct Node_A {
    T data;
    std::atomic<int> count;
    std::atomic<Node_A<T>*> next;

    Node_A(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};


#endif