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
    bool marked;

    Node(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};

#endif