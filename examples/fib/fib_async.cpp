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
    std::vector<std::future<int>> results;

    for (int i = 0; i < num_threads; ++i)
        results.emplace_back(std::async(std::launch::async, fib, i));

    for (auto& result : results)
        std::cout << result.get() << std::endl;
}
