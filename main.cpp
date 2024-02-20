#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "CMSet.hpp"


//This is just to stimulate a high-contention scenario
// We randomly pick between adding, removing, counting and containment checking
//We are just attempting to test the given implementations ability to handle concurrent modifications/stability
// This is not a performance test, we have included random sleep times to stimulate work
template<typename CMSetType>
void run_stress_test(CMSetType& cmset, int num_threads, int num_ops) {

    float operations_per_thread = num_ops / num_threads;

    auto thread_operation = [&](int thread_id) {
        for (int i = 0; i < operations_per_thread; ++i) {
            int method_choice = rand() % 4; // randomly chooses between add, remove etc etc
            int value = rand() % 100; //operate on values between 0 and 99

            switch (method_choice) {
                case 0:
                    cmset.add(value);
                    break;
                case 1:
                    cmset.remove(value);
                    break;
                case 2:
                    cmset.contains(value);
                    break;
                case 3:
                    cmset.count(value);     
                    break;
                    
            }

            //for fun, we introduce random sleeping, just to simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
    };

    std::vector<std::thread> threads;

    auto start_time = std::chrono::high_resolution_clock::now();

    //creating threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_operation, i));
    }

    //joining threads
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;

    std::cout << "Stress test successfully completed. (" << elapsed_time.count() << " milliseconds)" << std::endl;


}

// more controlled testing, compared to the stress test which is random operations
template<typename CMSetType>
void run_benchmarking_scenario(CMSetType& cmset, int num_threads, int num_ops, int read_percentage, int write_percentage) {

    float operations_per_thread = num_ops / num_threads;
    std::vector<std::thread> threads;

    //lamdba function easier to write than regular function
    //computes the ratio of read operations to writing operations, and executes them
    auto thread_operation = [&](int thread_id) {
        for (int i = 0; i < operations_per_thread; ++i) {
            if (i < operations_per_thread * read_percentage) {
                cmset.contains(rand() % 100); //operate on values between 0 and 99
            } else {
                cmset.add(rand() % 100);
            }
        }
    };

    auto start_time = std::chrono::high_resolution_clock::now();

    //creating threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_operation, i));
    }

    //joining threads
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;

    std::cout << "Test has completed in " << elapsed_time.count() << " milliseconds." << std::endl;
    std::cout << "Number of Threads: " << num_threads << std::endl;
    std::cout << "Throughput (ops/sec): " << num_ops/elapsed_time.count() << std::endl;


}

int main() {

    CMSet_Lock<int> cmset_lock;
    CMSet_O<int> cmset_o;
    CMSet_Lock_Free<int> cmset_lf;

    int num_threads = 10; //Ryzen 7 5800X has 8-cores, so we will do x2
    int num_ops = 1000;

    run_stress_test(cmset_lf, num_threads, num_ops);


    // float read_percentage = 0.7; //70% read
    // float write_percentage = 0.3; //30% write

    // run_benchmarking_scenario(cmset_lock, num_threads, num_ops, read_percentage, write_percentage);
    // run_benchmarking_scenario(cmset_o, num_threads, num_ops, read_percentage, write_percentage);
    // run_benchmarking_scenario(cmset_lf, num_threads, num_ops, read_percentage, write_percentage);


    

    return 0;

}