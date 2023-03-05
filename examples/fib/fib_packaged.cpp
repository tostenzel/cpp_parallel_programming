#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

int fib(int n) {
    int a_0 = 0;
    int a_1 = 1;
    for (int i = 0; i < n; ++i) {
        const int tmp = a_0;
        a_0 = a_1;
        a_1 += tmp;
    }

    return a_0;
}

int main() {
    static const int num_threads = 32;
    std::vector<std::thread> threads;
    std::vector<std::future<int>> results;

    for (int i = 0; i < num_threads; ++i) {
        auto task = std::packaged_task<int(int)>(fib);
        results.emplace_back(task.get_future());
        threads.emplace_back(std::move(task), i);
    }

    for (auto& result : results)
        std::cout << result.get() << std::endl;

    for (auto& thread : threads)
        thread.join();
}
