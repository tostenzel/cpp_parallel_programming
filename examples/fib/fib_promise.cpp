#include <future>
#include <iostream>
#include <thread>
#include <vector>

void fib(int n, std::promise<int>&& result) {
    int a_0 = 0;
    int a_1 = 1;
    for (int i = 0; i < n; ++i) {
        const int tmp = a_0;
        a_0 = a_1;
        a_1 += tmp;
    }

    result.set_value(a_0);
}

int main() {
    static const int num_threads = 32;
    std::vector<std::thread> threads;
    std::vector<std::future<int>> results;

    for (int i = 0; i < num_threads; ++i) {
        std::promise<int> promise;
        results.emplace_back(promise.get_future());
        threads.emplace_back(fib, i, std::move(promise));
    }

    for (auto& result : results)
        std::cout << result.get() << std::endl;

    for (auto& thread : threads)
        thread.join();
}
