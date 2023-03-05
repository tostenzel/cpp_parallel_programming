#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

template<class F, class... Args>
auto make_task(F&& f, Args&&... args) {
    auto aux = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    return std::packaged_task<std::invoke_result_t<F, Args...>(void)>(aux);
}

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
        auto task = make_task(fib, i);
        results.emplace_back(task.get_future());
        threads.emplace_back(std::move(task));
    }

    for (auto& result : results)
        std::cout << result.get() << std::endl;

    for (auto& thread : threads)
        thread.join();
}
