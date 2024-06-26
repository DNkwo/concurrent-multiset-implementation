//Concurrent Multi-set - Linked List Implementation

#ifndef CMSet_HPP
#define CMSet_HPP

#include "Node.hpp"
#include <iostream>
#include <mutex>


//abstract class 'CMSet', concrete implementations derive this interface
template <typename T>
class CMSet {
    
    protected: //shared variables
    Node<T>* head = nullptr;

    public:
    CMSet() : head(nullptr) {} //constructor

    /*======= Abstract Methods ==========*/
    virtual bool contains (const T& element) = 0; //good practice to include the 'const' method signature
    virtual int  count (const T& element) = 0; //in other words, the multiplicity
    virtual void add (const T& element) = 0; //adding an element to the bag
    virtual bool remove (const T& element) = 0; //removing an element form the bag


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

        bool contains(const T& element) override {
            std::lock_guard<std::mutex> lock(mtx); //RAII-style, meaning that lock is unlocked once we leave scope


            //traverses the list, checking if the current node data matches the element data
            Node<T>* current = this->head; 
            while (current != nullptr) {
                if (current->data == element) {
                    return true;
                }
                current = current->next;
            }

            return false;
        }


        int count(const T& element) override {
            std::lock_guard<std::mutex> lock(mtx);

            Node<T>* current = this->head;
            while (current != nullptr) {
                if (current->data == element) {
                    return current->count;
                }
                current = current->next;
            }

            return 0; //else returns count 0
        }

        void add(const T& element) override {
            std::lock_guard<std::mutex> lock(mtx);

            Node<T>* current = this->head;
            while (current != nullptr) {
                if (current->data == element) { //if node that matches is found
                    current->count++; // return count+1
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

        // Destructor, destroys the object and deallocates all the nodes in the list
        ~CMSet_Lock() {
            std::lock_guard<std::mutex> lock(mtx); //also ensures exclusive access during cleanup.
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
        * It also checks if the predecessor node actually points to the current node (this may have also been modified)
        */
        bool is_valid(const Node<T>* pred, const Node<T>* current) const {
            Node<T>* t = this->head;

            while (t != nullptr) {
                if (t == current) {
                    if (pred == nullptr) return true; //if pred is null, then we skip the pred->next = current check
                    return pred->next == t; //checks if pred->next is still referring to the current
                }
                t = t->next;
            }

            return false;
        }

    public:

        CMSet_O() : CMSet<T>() {} //constructor

        bool contains(const T& element) override {
            Node<T>* current = this->head;
            Node<T>* pred = nullptr;

            while (true) {
                current = this->head;
                pred = nullptr;

                while (current != nullptr) {
                    if (current->data == element) { // if element are equal
                        if (pred != nullptr) { pred->mtx.lock(); }
                        current->mtx.lock();

                        if (is_valid(pred, current)) { //check if node is valid (not been deleted)
                            if (pred != nullptr) { pred->mtx.unlock(); }
                            current->mtx.unlock();
                            return true; // element is found and valid
                        }   
                        //if node is not valid, probably deleted during traversal, so unlock and retry
                        if (pred != nullptr) { pred->mtx.unlock(); }
                        current->mtx.unlock();
                        break; //break from loop, in order to retry
                
                    }
                    pred = current;
                    current = current->next;
                } 
            
                return false; //Element is not found
            }
        }


        //Notes for report:
        // includes tracking a 'pred' node and then locking the predecessor ensures that no other thread can modify the 'next' pointer of the predecessor at the same time,
        // also list integrity is maintained this way, preventing dangling pointers or broken chains, this could happen if another thread concurrently changes the list structure.
        void add(const T& element) override {

            Node<T>* pred = nullptr; //predecessor
            Node<T>* current;

            while (true) { //keep on re-trying, if the node is invalid when writing
                pred = nullptr;
                current = this->head;

                while (current != nullptr) {
                    if (current->data == element) { // if element are equal
                        if (pred != nullptr)  { pred->mtx.lock(); }
                        current->mtx.lock();

                        if (is_valid(pred, current)) { //check if node is valid (not been deleted)
                            // update the node as it exists and is valid 
                            current->count++;
                            if (pred != nullptr) { pred->mtx.unlock(); }
                            current->mtx.unlock();
                            return;
                        }

                        // if current node or predecessor is not valid, unlock and retry
                        if (pred != nullptr) { pred->mtx.unlock(); }
                        current->mtx.unlock();
                        break; //break from loop, inorder to retry
                    }

                    // move to the next node, updating the predecessor and current pointers
                    if (pred != nullptr) { pred->mtx.unlock(); } // unlock the previous predecessor

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


        //Notes for report: Recognising that there's no modification of data structure, so concerns about locking 'pred' that we did in add/remove are not as important.
        //but we still have to guarantee that the current node being read from has not been concurrently modified or deleted whilst accessing
        int count(const T& element) override {
            Node<T>* current = this->head;
            Node<T>* pred = nullptr;

            while (true) {
                current = this->head;
                pred = nullptr;

                while (current != nullptr) {
                    if (current->data == element) { // if element are equal
                        if (pred != nullptr) { pred->mtx.lock(); }
                        current->mtx.lock();

                        if (is_valid(pred, current)) { //check if node is valid (not been deleted)
                            int count = current->count; //note: this is before the unlocks, placing it after exposes us to race conditions
                            if (pred != nullptr) { pred->mtx.unlock(); }
                            current->mtx.unlock();
                            return count; // element is found and valid
                        }   
                        //if node is not valid, probably deleted during traversal, so unlock and retry
                        if (pred != nullptr) { pred->mtx.unlock(); }
                        current->mtx.unlock();
                        break; //break from loop, in order to retry
                
                    }
                    pred = current;
                    current = current->next;
                } 
            
                return 0; //Element is not found
            }
        }


        bool remove(const T& element) override {
            while (true) { // keep on re-trying, if the node is invalid when writing
                Node<T>* current = this->head;
                Node<T>* pred = nullptr;

                while (current != nullptr) {
                    if (current->data == element) {
                        if (pred != nullptr) { pred->mtx.lock(); }
                        current->mtx.lock();

                        if (is_valid(pred, current)) { // check if node is valid 
                            if (current->count > 1) { // if multiplicity/count is greater than 1, decrement by 1
                                current->count--;
                                if (pred != nullptr)  { pred->mtx.unlock(); }
                                current->mtx.unlock();
                                return true;
                            } else {
                                if (pred == nullptr) { // if there is no pred node, set the 'head' to the succeeding node
                                    this->head = current->next;
                                } else {
                                    pred->next = current->next; // pass pred's next value to current's succeeding node
                                }
                                if (pred != nullptr) { pred->mtx.unlock(); }
                                current->mtx.unlock();
                                // delete current; // physically remove current
                                return true;
                            }
                        } else {
                            if (pred != nullptr) { pred->mtx.unlock(); }
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
 * Lock-free algorithm
 *  w/Lazy Synchronisation
*/

template <typename T>
class CMSet_Lock_Free : public CMSet<T> {

    private:

        std::atomic<Node_A<T>*> head = nullptr;

        //mark a node as logically removed
        //note to self: the reinterpret_cast converts the next pointer to an unsigned integer of the same size, allowing bitwise operations on it
        // possible due to modern architecturers having a 2-byte boundary for pointers :) 
        bool mark_node_for_deletion(Node_A<T>* node) {
            Node_A<T>* expected_next = node->next.load(std::memory_order_relaxed);
            Node_A<T>* marked_next = reinterpret_cast<Node_A<T>*>(reinterpret_cast<uintptr_t>(expected_next) | 1); // sets the LSB to be 1 
            return node->next.compare_exchange_strong(expected_next, marked_next, std::memory_order_release, std::memory_order_relaxed); // CAS checking if node->next is still expected_next
        }

        //checks if there is a '1' set on the next pointer.
        bool is_marked_for_deletion(Node_A<T>* node) {
            Node_A<T>* next_node = node->next.load(std::memory_order_relaxed);
            return reinterpret_cast<uintptr_t>(next_node) & 1; //checks if LSB has been set 
        }

        //used to unmark the next pointer, so we can use it for other operations (such as traversing)
        Node_A<T>* clean_marked_bit(Node_A<T>* node_marked) {
            return reinterpret_cast<Node_A<T>*>(reinterpret_cast<uintptr_t>(node_marked) & ~uintptr_t(1)); //clears LSB, of the int representation of pointer. 
        }

    public:

        CMSet_Lock_Free() : CMSet<T>() {} //constructor


        //Notes for report: WAIT- FREE!
        bool contains(const T& element) override {
            Node_A<T>* current = this->head.load(std::memory_order_acquire);

            while (current != nullptr) {
                if (!is_marked_for_deletion(current)) { // check if the node is marked for deletion
                    if (current->data == element) {
                        return true; //elemenet has been found
                    }
                }

                 //load next node out of comparison to avoid data race
                Node_A<T>* next = current->next.load(std::memory_order_acquire);
                current = clean_marked_bit(next); //move to the next node, by unmarking the next pointer
            } 
        
            return false; //Element is not found
        }

        void add(const T& element) override {
            while (true) { //keep on re-trying, if the node is invalid when writing
                Node_A<T>* current = this->head.load(std::memory_order_acquire);



                while (current != nullptr) {
                    if (current->data == element) {
                        // atomically increase count, since element found
                        int cnt = current->count.load(std::memory_order_acquire);
                        if (current->count.compare_exchange_weak(cnt, cnt + 1)) {
                            return; //success
                        } 

                        //if CAS fails (ue to another thread interaction), the loop will restart anyway and try agian
                    } 
                    
                     //load next node out of comparison to avoid data race
                    Node_A<T>* next = current->next.load(std::memory_order_acquire);
                    current = clean_marked_bit(next); //move to the next node, by unmarking the next pointer
                }


                //prepare new node for insertion
                Node_A<T>* newNode = new Node_A<T>(element);
                Node_A<T>* expected = this->head.load(std::memory_order_acquire);
                newNode->next.store(expected, std::memory_order_relaxed);


    
                //attempt to insert new node at head
                if (head.compare_exchange_weak(expected, newNode, std::memory_order_release, std::memory_order_relaxed)) {
                    return; //success
                }

                delete newNode; //once again if CAS fails, another thread must have interfered, so delete newnode and retry ;-;
            }

        }

        //Notes for report: WAIT- FREE!
        int count(const T& element) override {
            Node_A<T>* current = this->head.load(std::memory_order_acquire);
                                    
            while (current != nullptr) {

                if (!is_marked_for_deletion(current)) { // if element are equal and is not marked true
                    if (current->data == element) {
                        int cnt = current->count.load(std::memory_order_acquire);
                        return cnt;
                    }
                }

                Node_A<T>* next = current->next.load(std::memory_order_acquire);
                current = clean_marked_bit(next); //move to the next node, by unmarking the next pointer
            }

            return 0;
        }


        //Notes for report: leverages logical removals, for wait-free contains() and count()
        bool remove(const T& element) override {
            while (true) { // keep on re-trying, if the node is invalid when writing
                Node_A<T>* current = this->head.load(std::memory_order_acquire);
                Node_A<T>* pred = nullptr;
                Node_A<T>* succ = nullptr; //used to check future nodes in the list for logical deletion mark

                while (current != nullptr) {
                    succ = current->next.load(std::memory_order_acquire); 

                    if (succ != nullptr && is_marked_for_deletion(succ)) { //first, its important to check if successor node is marked for deletion
                        Node_A<T>* succ_next = clean_marked_bit(succ->next.load(std::memory_order_acquire));
                        if (current->next.compare_exchange_strong(succ, succ_next)) {
                            //possible memory reclamation?
                        } 
                    } else { //if succ node is not marked
                        if (current->data == element) {
                            int cnt = current->count.load(std::memory_order_acquire);
                            if (cnt > 1) { //if multiplicity/count is greater than 1, decrement by 1
                                current->count.compare_exchange_weak(cnt, cnt - 1);
                                return true; //successful removal
                            }

                            if(!mark_node_for_deletion(current)) {
                                break; //CAS inside Mark method failed, restart loop to retry
                            }

                            // otherwise if CAS success, physical unlink
                            if (pred->next.compare_exchange_strong(current, succ)) {
                                delete current;
                                return true;
                            }

                            break; //CAS failed for physical unlink, restart loop to retry
                        }
                    }
                    // continue traversing linked list
                    pred = current;
                    current = clean_marked_bit(succ); //move to the next node, by unmarking the next pointer
                }


                // If current is nullptr, we've not found the element, so return false
                if (current == nullptr) {
                    return false; // false indicating element not found
                }
            }
        }
}; 

#endif