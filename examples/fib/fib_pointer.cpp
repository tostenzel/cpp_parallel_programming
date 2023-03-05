#include <iostream>
#include <thread>
#include <vector>

void fib(int n, int* result) {
    int a_0 = 0;
    int a_1 = 1;
    for (int i = 0; i < n; ++i) {
        const int tmp = a_0;
        a_0 = a_1;
        a_1 += tmp;
    }

    *result = a_0;
}

int main() {
    static const int num_threads = 32;
    std::vector<std::thread> threads;

    std::vector<int> results(num_threads, 0);

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(fib, i, &results[i]);

    for (auto& thread : threads)
        thread.join();

    for (const auto& result: results)
        std::cout << result << std::endl;
}
