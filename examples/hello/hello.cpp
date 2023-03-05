#include <thread>
#include <iostream>
#include <vector>

// this function will be called by the threads (should be void)
int say_hello(int id) {
    std::cout << "Hello from thread: " << id << std::endl;
    return id;
}

// this runs in the master thread
int main() {
    static const int num_threads = 4;
    std::vector<std::thread> threads;

    for (int id = 0; id != num_threads; ++id) threads.emplace_back(say_hello, id);
    for (auto& thread: threads) thread.join();
}

