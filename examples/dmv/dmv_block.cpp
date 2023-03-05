#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include "pp/util.h"

template<class V>
void init(size_t rows, size_t cols, V* A, V* x) {
    for (size_t row = 0; row < rows; ++row)
        for (size_t col = 0; col < cols; ++col)
            A[row*cols + col] = row >= col ? 1 : 0;

    for (size_t col = 0; col < cols; ++col)
        x[col] = col;
}

template<class V>
void dmv(size_t rows, size_t cols, const V* A, const V* x, V* b, size_t num_threads) {
    auto block = [=](size_t i) {
        size_t chunk = ceil_div(rows, num_threads);
        size_t lower = i * chunk;
        size_t upper = std::min(lower+chunk, rows);

        for (size_t row = lower; row < upper; ++row) {
            V acc = 0;
            for (size_t col = 0; col < cols; ++col)
                acc += A[row*cols + col] * x[col];
            b[row] = acc;
        }
    };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) threads.emplace_back(block, i);
    for (auto& thread : threads) thread.join();
}

int main(int argc, char** argv) {
    if (argc != 2) return EXIT_FAILURE;

    const size_t num_threads = std::stoi(argv[1]);
    std::cout << "using " << num_threads << " threads" << std::endl;

    const size_t rows = 1ul << 15; // 2^15 = 32768
    const size_t cols = 1ul << 15; // 2^15 = 32768
    auto A = std::make_unique<double[]>(rows * cols);
    auto x = std::make_unique<double[]>(cols);
    auto b = std::make_unique<double[]>(rows);

    init(rows, cols, A.get(), x.get());
    auto t = start_timer();
    dmv(rows, cols, A.get(), x.get(), b.get(), num_threads);
    stop_timer(t, "dmv");

    // check if correct
    for (size_t row = 0; row < rows; ++row)
        if (b[row] != row*(row+1)/2) std::cout << "error at position " << row << std::endl;
    return 0;
}
