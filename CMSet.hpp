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
            Node<T>* pred = nullptr;
            while (current != nullptr) {
                if (current->data == element) {
                    if (current->count > 1) { //if multiplicity/count is greater than 1, we just decrement by 1
                        current->count--;
                        return true;
                    } else {
                        if (pred == nullptr) { //if there is no pred node, we set the 'head' to the succeeding node
                            this->head = current->next;
                        } else {
                            pred->next = current->next; // pass pred's next value to current's succeeding node
                        }
                        delete current; // physically remove current
                        return true;
                    }
                }
                //continue traversing linked list
                pred = current;
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
 * Optimistic Synchronization
*/

template <typename T>
class CMSet_O : public CMSet<T> {

    private:

        /**
        * Method validates multi-state by checking if the node is reachable from the head (not deleted)
        * Notes for report: appreciating that this method might not the most sufficient, due to linear time complexity
        */
        bool isValid(const Node<T>* element) const {
            Node<T>* current = this->head;


            while (current != nullptr) {
                if (current == element) {
                    return true;
                }
                current = current->next;
            }

            return false;
        }

    public:

        CMSet_O() : CMSet<T>() {} //constructor

        bool contains(const T& element) const override {
            Node<T>* current = this->head;

            while (current != nullptr) {
                if (current->data == element) { // if element are equal
                    current->mtx.lock();
                    if (isValid(current)) { //check if node is valid (not been deleted)
                        current->mtx.unlock();
                        return true; // element is found and valid
                    } else { //if node is not valid, probably deleted during traversal, so just ignore and continue traversal
                        current->mtx.unlock();
                    }
                }
                current = current->next;
            } 
        
            return false; //Element is not found
        }


        //Notes for report:
        // includes predecessor tracking and then locking the predecessor ensures that no toher thread can modify the 'next' pointer of the predecessor at the same time,
        // also list integrity is maintained this way, preventing dangling pointers or broken chains, this could happen if another thread concurrently changes the list structure.

        void add(const T& element) override {

            Node<T>* pred = nullptr; //predecessor
            Node<T>* current;

            while (true) { //keep on re-trying, if the node is invalid when writing
                pred = nullptr;
                current = this->head;

                while (current != nullptr) {
                    if (current->data == element) { // if element are equal
                        if (pred != nullptr) pred->mtx.lock();
                        current->mtx.lock();

                        if (isValid(current)) { //check if node is valid (not been deleted)
                            if (pred == nullptr || isValid(pred)) { //we also check if predeccesor is also valid, or if we are at the head
                                // update the node as it exists and is valid 
                                current->count++;
                                if (pred != nullptr) pred->mtx.unlock();
                                current->mtx.unlock();
                                return;
                            }
                        }

                        // if current node or predecessor is not valid, unlock and retry
                        if (pred != nullptr) pred->mtx.unlock();
                        current->mtx.unlock();
                        break; //break from loop, inorder to retry
                    }

                    // move to the next node, updating the predecessor and current pointers
                    if (pred != nullptr) pred->mtx.unlock(); // unlock the previous predecessor
                    pred = current;
                    current = current->next;
                }


    
                if (current == nullptr) { //if element does not exist

                    //attempts to add new node at beginning 
                    Node<T>* newNode = new Node<T>(element);
                    if(this->head != nullptr) {
                        this->head->mtx.lock(); 
                    }//lock head
                    newNode->next = this->head;
                    this->head = newNode; 

                    if(newNode->next != nullptr) {
                        newNode->next->mtx.unlock();
                    }
                    return; 

                }

            }

        }


        //Notes for report: Recognising that there's no modification of data structure, so concerns about locking 'pred' that we did in add/remove not as important.
        //but we still have to guarantee that the current node being read from has not been concurrently modified or deleted whilst accessing
        int count(const T& element) const override {

            Node<T>* current = this->head;
                                    
            while (current != nullptr) {

                if (current->data == element) { // if element are equal
                    current->mtx.lock();
                    if (isValid(current)) { //check if node is valid (not been deleted)
                        int count = current->count; //ensures that the unlock comes AFTER accessing this shared count
                        current->mtx.unlock();
                        return count;
                    } else {
                        current->mtx.unlock(); //if current node is invalid, unlock and continue the search/traversal, no need to retry from the beginning
                        break;
                    }
                }
                current = current->next;
            }

            return 0;
        }

        // OLD-INEFFICIENT METHOD - It is kind of pointless to restart the traversal if it encounters an invalid node, not necessary for just counting occurences
        // int count(const T& element) const override {

        //     while (true) { //keep on re-trying, if the node is invalid when writing
        //         Node<T>* current = this->head;
                                    
        //         while (current != nullptr) {

        //             if (current->data == element) { // if element are equal
        //                 current->mtx.lock();
        //                 if (isValid(current)) { //check if node is valid (not been deleted)
        //                     int count = current->count; //ensures that the unlock comes AFTER accessing this shared count
        //                     current->mtx.unlock();
        //                     return count;
        //                 } else {
        //                     current->mtx.unlock();
        //                     break;
        //                 }
        //             }
        //             current = current->next;
        //         }

        //         if(current == nullptr) {
        //             return 0; // does not exist
        //         }


        //     }
            
        // }

        bool remove(const T& element) override {
            while (true) { // keep on re-trying, if the node is invalid when writing
                Node<T>* current = this->head;
                Node<T>* pred = nullptr;

                while (current != nullptr) {
                    if (current->data == element) {
                        if (pred != nullptr) pred->mtx.lock();
                        current->mtx.lock();

                        if (isValid(current)) { // check if node is valid (not been deleted)
                            if (current->count > 1) { // if multiplicity/count is greater than 1, decrement by 1
                                current->count--;
                                if (pred != nullptr) pred->mtx.unlock();
                                current->mtx.unlock();
                                return true;
                            } else {
                                if (pred == nullptr) { // if there is no pred node, set the 'head' to the succeeding node
                                    this->head = current->next;
                                } else {
                                    pred->next = current->next; // pass pred's next value to current's succeeding node
                                }
                                if (pred != nullptr) pred->mtx.unlock();
                                current->mtx.unlock();
                                delete current; // physically remove current
                                return true;
                            }
                        } else {
                            if (pred != nullptr) pred->mtx.unlock();
                            current->mtx.unlock();
                            break; // Invalid node, try again
                        }
                    }
                    // continue traversing linked list
                    pred = current;
                    current = current->next;
                    if (pred != nullptr && pred != this->head) {
                        pred->mtx.unlock();  // Optimisation: unlock the previous node early to reduce additional lock contention
                    }
                }

                if (pred != nullptr && pred != this->head) {
                    pred->mtx.unlock(); //ensure the predecessor is unlocked if we exit the loop
                }

                // If current is nullptr, we've not found the element, so return false
                if (current == nullptr) {
                    return false; // false indicating element not found
                }
            }
        }
}; 


/**
 * Lazy Synchronisation
*/

template <typename T>
class CMSet_Lock_Free : public CMSet<T> {

    private:

        bool isValid(const Node<T>* pred, const Node<T>* current) const {
            if(pred == nullptr) { //if predecessor is null
                return !current.marked;
            }

            return !pred->marked && !current.marked && pred.next == current;
        }

    public:

        bool contains(const T& element) const override {
            
            
        }

        void add(const T& element) override {

            Node<T>* pred = nullptr; //predecessor
            Node<T>* current;

            while (true) { //keep on re-trying, if the node is invalid when writing
                pred = nullptr;
                current = this->head;

                while (current != nullptr) {
                    if (current->data == element) { // if element are equal
                        if (pred != nullptr) pred->mtx.lock();
                        current->mtx.lock();

                        if (isValid(pred, current)) { //check if node is valid (not been deleted) (checks for marked)
                            // update the node as it exists and is valid 
                            current->count++;
                            if (pred != nullptr) pred->mtx.unlock();
                            current->mtx.unlock();
                            return;
                        }

                        // if current node or predecessor is not valid, unlock and retry
                        if (pred != nullptr) pred->mtx.unlock();
                        current->mtx.unlock();
                        break; //break from loop, inorder to retry
                    }

                    // move to the next node, updating the predecessor and current pointers
                    if (pred != nullptr) pred->mtx.unlock(); // unlock the previous predecessor
                    pred = current;
                    current = current->next;
                }


    
                if (current == nullptr) { //if element does not exist

                    //attempts to add new node at beginning 
                    Node<T>* newNode = new Node<T>(element);
                    if(this->head != nullptr) { //lock head
                        this->head->mtx.lock(); 
                    }


                    newNode->next = this->head;
                    this->head = newNode; 

                    if(newNode->next != nullptr) {
                        newNode->next->mtx.unlock(); //unlock old head
                    }
                    return; 

                }

            }

        }

        
        int count(const T& element) const override {

        }

        bool remove(const T& element) override {
            while (true) { // keep on re-trying, if the node is invalid when writing
                Node<T>* current = this->head;
                Node<T>* pred = nullptr;

                while (current != nullptr) {
                    if (current->data == element) {
                        if (pred != nullptr) pred->mtx.lock();
                        current->mtx.lock();

                        if (isValid(pred, current)) { // check if node is valid (not been deleted)
                            if (current->count > 1) { // if multiplicity/count is greater than 1, decrement by 1
                                current->count--;
                                if (pred != nullptr) pred->mtx.unlock();
                                current->mtx.unlock();
                                return true;
                            } else {
                                if (pred == nullptr) { // if there is no pred node, set the 'head' to the succeeding node
                                    this->head = current->next;
                                } else {
                                    pred->next = current->next; // pass pred's next value to current's succeeding node
                                }
                                if (pred != nullptr) pred->mtx.unlock();
                                current->mtx.unlock();
                                delete current; // physically remove current
                                return true;
                            }
                        } else {
                            if (pred != nullptr) pred->mtx.unlock();
                            current->mtx.unlock();
                            break; // Invalid node, try again
                        }
                    }
                    // continue traversing linked list
                    pred = current;
                    current = current->next;
                    if (pred != nullptr && pred != this->head) {
                        pred->mtx.unlock();  // Optimisation: unlock the previous node early to reduce additional lock contention
                    }
                }

                if (pred != nullptr && pred != this->head) {
                    pred->mtx.unlock(); //ensure the predecessor is unlocked if we exit the loop
                }

                // If current is nullptr, we've not found the element, so return false
                if (current == nullptr) {
                    return false; // false indicating element not found
                }
            }
        }
}; 

#endif