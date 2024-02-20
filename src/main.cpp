#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>

#include "CMSet.hpp"

template<typename CMSetType>
void run_stress_test_1(CMSetType& cmset, int num_threads, int num_ops) {

    float operations_per_thread = num_ops / num_threads;

    auto thread_operation = [&](int thread_id) {
        for (int i = 0; i < operations_per_thread; ++i) {
            
            int num = thread_id * num_ops + i;
            cmset.add(num);

            assert(cmset.contains(num)); //verify number was added

            int count = cmset.count(num);
            assert(count == 1);

            if (i % 10 == 0) {
                assert(cmset.remove(num));
                assert(!cmset.contains(num)); //verify removal
            }
        }
    };

    std::vector<std::thread> threads;


    //creating threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_operation, i));
    }

    //joining threads
    for (auto& thread : threads) {
        thread.join();
    }


    std::cout << "Stress test 1 successfully completed." << std::endl;


}


//This is just to stimulate a high-contention scenario
// We randomly pick between adding, removing, counting and containment checking
//We are just attempting to test the given implementations ability to handle concurrent modifications/stability
// This is not a performance test, we have included random sleep times to stimulate work
template<typename CMSetType>
void run_stress_test_2(CMSetType& cmset, int num_threads, int num_ops) {

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

    std::cout << "Stress test 2 successfully completed. (" << elapsed_time.count() << " milliseconds)" << std::endl;


}

// more controlled testing, compared to the stress test which is random operations
template<typename CMSetType>
void run_benchmarking_scenario(CMSetType& cmset, int num_threads, int num_ops, int read_percentage, int write_percentage) {

    float operations_per_thread = num_ops / num_threads;
    std::vector<std::thread> threads;
    std::vector<double> latencies; //store latencies, average latency per thread
    latencies.resize(num_threads); //changing size of vector to match thread numbers

    //lamdba function easier to write than regular function
    //computes the ratio of read operations to writing operations, and executes them
    auto thread_operation = [&](int thread_id) {
        for (int i = 0; i < operations_per_thread; ++i) {


            auto start_op = std::chrono::high_resolution_clock::now(); //starts timer for a specific operation


            if (i < operations_per_thread * read_percentage) {
                cmset.contains(rand() % 100); //operate on values between 0 and 99
            } else {
                cmset.add(rand() % 100);
            }


            auto end_op = std::chrono::high_resolution_clock::now(); //end of timer for specific operation
            std::chrono::duration<double, std::milli> op_time = end_op - start_op; 

            latencies[thread_id] += op_time.count(); //add to accumulated operation times for this thread
        }
        latencies[thread_id] /= operations_per_thread; // calculates average latency per thread
    };

    auto start_time = std::chrono::high_resolution_clock::now(); //start timer for the entire benchmark test

    //creating threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_operation, i));
    }

    //joining threads
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now(); //end timer for the entire benchmark test
    std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;

    //generate latency
    double total_latency = 0;
    for (const double& latency : latencies) {
        total_latency += latency; //sums up all latencies of each thread 
    }

    std::cout << "Test has completed in " << elapsed_time.count() << " milliseconds." << std::endl;
    std::cout << "Number of Threads: " << num_threads << std::endl;
    std::cout << "Total Operations: " << num_ops << std::endl;
    std::cout << "Throughput (ops/sec): " << (num_ops * 1000) / elapsed_time.count() << std::endl;
    std::cout << "Average Latency (ms/ops): " << total_latency / num_threads << std::endl;


}

int main() {

    CMSet_Lock<int> cmset_lock;
    CMSet_O<int> cmset_o;
    CMSet_Lock_Free<int> cmset_lf;

    int num_threads = 10; //Ryzen 7 5800X has 8-cores, so we will do x2
    int num_ops = 400;

    run_stress_test_1(cmset_o, num_threads, num_ops);


    // float read_percentage = 0.7; //70% read
    // float write_percentage = 0.3; //30% write

    // run_benchmarking_scenario(cmset_lock, num_threads, num_ops, read_percentage, write_percentage);
    // run_benchmarking_scenario(cmset_o, num_threads, num_ops, read_percentage, write_percentage);
    // run_benchmarking_scenario(cmset_lf, num_threads, num_ops, read_percentage, write_percentage);


    

    return 0;

}