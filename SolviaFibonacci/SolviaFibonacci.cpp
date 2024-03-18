#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

using namespace std;
using namespace std::chrono;

atomic<bool> keepRunning(true);
mutex coutMutex;

// Dummy computation function
void heavyComputation() {
    volatile double result = 1.0; // Use volatile to prevent optimizations that could skip calculations
    for (int i = 1; i <= 10000; ++i) {
        result = sin(result) * tan(result) + sqrt(result);
    }
}


// Stress test function that runs for a specified duration and periodically reports progress
void stressTest(int id, unsigned runForSeconds) {
    auto startTime = high_resolution_clock::now();
    unsigned long long iterations = 0;

    while (keepRunning) {
        heavyComputation();
        iterations++;

        if (runForSeconds > 0) {
            auto currentTime = high_resolution_clock::now();
            if (duration_cast<seconds>(currentTime - startTime).count() >= runForSeconds) {
                keepRunning = false;
            }
        }

        // Periodic reporting
        if (iterations % 1000 == 0) {
            lock_guard<mutex> lock(coutMutex);
            cout << "Thread " << id << ": Iterations = " << iterations << endl;
        }
    }

    lock_guard<mutex> lock(coutMutex);
    cout << "Thread " << id << " completed: Total iterations = " << iterations << endl;
}

int main(int argc, char* argv[]) {
    unsigned runForSeconds = 0; // Default: run indefinitely

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--run-for-seconds" && i + 1 < argc) {
            runForSeconds = stoi(argv[++i]);
        }
    }

    int threadsCount = thread::hardware_concurrency();
    vector<thread> threads;

    // Launch threads
    for (int i = 0; i < threadsCount; ++i) {
        threads.push_back(thread(stressTest, i, runForSeconds));
    }

    // Wait for all threads to complete
    for (auto& th : threads) {
        th.join();
    }

    cout << "All threads completed." << endl;
    return 0;
}
