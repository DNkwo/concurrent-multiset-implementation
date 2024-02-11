#ifndef NODE_HPP
#define NODE_HPP

template <typename T>
struct Node {
    T data;
    int count;
    Node* next;

    Node(T data, int count = 1) : data(data), count(count), next(nullptr) {} //node constructor

};

#endif