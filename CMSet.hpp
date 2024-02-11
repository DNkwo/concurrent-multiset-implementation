//Concurrent Multi-set - Linked List Implementation

#ifndef CMSet_HPP
#define CMSet_HPP

#include "Node.hpp"
#include <iostream>
#include <mutex>


//abstract class 'CMSet'
template <typename T>
class CMSet {
    
    protected: //shared variables
    Node<T>* head = nullptr;

    public:
    CMSet() : head(nullptr) {} //constructor

    /*======= Abstract Methods ==========*/
    virtual bool contains (const T& element) const = 0; //good practice to include the 'const' method signature
    virtual int  count (const T& element) const = 0; //in other words, the multiplicity
    virtual void add (const T& element) = 0;
    virtual bool remove (const T& element) = 0;


    virtual ~CMSet() {} //destructor

};


/**
 * Single Lock Implementation
 * (Coarse-grained Synchronisation)
*/
template <typename T>
class CMSet_Lock : public CMSet<T> {
    private:
        mutable std::mutex mtx; // mutex to protect linked list
    public:

        CMSet_Lock() : CMSet<T>() {} //constructor

        bool contains(const T& element) const override {
            std::lock_guard<std::mutex> lock(mtx); //RAII-style, meaning that lock is unlocked once we leave scope

            Node<T>* current = this->head;
            while (current != nullptr) {
                if (current->data == element) {
                    return true;
                }
                current = current->next;
            }

            return false;
        }


        int count(const T& element) const override {
            std::lock_guard<std::mutex> lock(mtx);

            Node<T>* current = this->head;
            size_t c = 0;
            while (current != nullptr) {
                if (current->data == element) {
                    return current->count;
                }
                current = current->next;
            }

            return 0; //else returns 0
        }

        void add(const T& element) override {
            std::lock_guard<std::mutex> lock(mtx);

            Node<T>* current = this->head;
            while (current != nullptr) {
                if (current->data == element) {
                    current->count++;
                    return; 
                }
                current = current->next;
            }

            //if element does not exist
            Node<T>* newNode = new Node<T>(element);
            newNode->next = this->head;
            this->head = newNode; //new node at the front of the list
        }

        bool remove(const T& element) override {
            std::lock_guard<std::mutex> lock(mtx);

            Node<T>* current = this->head;
            Node<T>* prev = nullptr;
            while (current != nullptr) {
                if (current->data == element) {
                    if (current->count > 1) { //if multiplicity/count is greater than 1, we just decrement by 1
                        current->count--;
                        return true;
                    } else {
                        if (prev == nullptr) { //if there is no prev node, we set the 'head' to the succeeding node
                            this->head = current->next;
                        } else {
                            prev->next = current->next; // pass prev's next value to current's succeeding node
                        }
                        delete current; // physically remove current
                        return true;
                    }
                }
                //continue traversing linked list
                prev = current;
                current = current->next;
            }

            return false; //element not found, so return false
        }


        ~CMSet_Lock() {
            std::lock_guard<std::mutex> lock(mtx);
            Node<T>* current = this->head;
            while (current != nullptr) {
                Node<T>* next = current->next;
                delete current;
                current = next;
            }
        }
}; 



/**
 * Lock Free Implementation
 * (Fine-grained Synchronisation w/ Optimistic Synchronisation)
*/
template <typename T>
class CMSet_Lock_Free : public CMSet<T> {
    private:

    public:
}; 

#endif