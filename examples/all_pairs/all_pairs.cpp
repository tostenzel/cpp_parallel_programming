#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "pp/io.h"
#include "pp/util.h"

template<class T>
void sequential_all_pairs(const T* mnist, T* all_pairs, size_t rows, size_t cols) {
    // for all entries below the diagonal (i'=I)
    for (size_t i = 0; i < rows; ++i) {
        for (size_t I = 0; I <= i; ++I) {
            // compute squared Euclidean distance
            T acc = T(0);
            for (size_t j = 0; j < cols; ++j) {
                T residue = mnist[i * cols + j] - mnist[I * cols + j];
                acc += residue * residue;
            }

            // write Delta[i,i'] = Delta[i',i] = dist(i, i')
            all_pairs[i * rows + I] = all_pairs[I * rows + i] = acc;
        }
    }
}

template<class T>
void parallel_all_pairs(const T* mnist,
                        T* all_pairs,
                        size_t rows,
                        size_t cols,
                        size_t num_threads = 64,
                        size_t chunk_size  = 64 / sizeof(T)) {
    auto block_cyclic = [&](size_t id) {
        // precompute offset and stride
        const size_t off = id * chunk_size;
        const size_t str = num_threads * chunk_size;

        // for each block of size chunk_size in cyclic order
        for (size_t lower = off; lower < rows; lower += str) {
            // compute the upper border of the block (exclusive)
            const size_t upper = std::min(lower + chunk_size, rows);

            // for all entries below the diagonal (i'=I)
            for (size_t i = lower; i < upper; ++i) {
                for (size_t I = 0; I <= i; ++I) {
                    // compute squared Euclidean distance
                    T acc = T(0);
                    for (size_t j = 0; j < cols; ++j) {
                        T residue = mnist[i * cols + j] - mnist[I * cols + j];
                        acc += residue * residue;
                    }

                    // write Delta[i,i'] = Delta[i',i]
                    all_pairs[i * rows + I] = all_pairs[I * rows + i] = acc;
                }
            }
        }
    };

    std::vector<std::thread> threads;

    for (size_t id = 0; id < num_threads; ++id)
        threads.emplace_back(block_cyclic, id);

    for (auto& thread : threads)
        thread.join();
}

template<class T>
void dynamic_all_pairs(const T* mnist,
                       T* all_pairs,
                       size_t rows,
                       size_t cols,
                       size_t num_threads = 64,
                       size_t chunk_size  = 64 / sizeof(T)) {
    size_t global_lower = 0;
    std::mutex mutex;

    auto dynamic_block_cyclic = [&]() {
        // assume we have not done anything
        size_t lower = 0;

        // while there are still rows to compute
        while (lower < rows) {
            // update lower row with global lower row
            {
                std::scoped_lock lock(mutex);
                lower = global_lower;
                global_lower += chunk_size;
            }

            // compute the upper border of the block (exclusive)
            const size_t upper = std::min(lower + chunk_size, rows);

            // for all entries below the diagonal (i'=I)
            for (size_t i = lower; i < upper; ++i) {
                for (size_t I = 0; I <= i; ++I) {
                    // compute squared Euclidean distance
                    T acc = T(0);
                    for (size_t j = 0; j < cols; ++j) {
                        T residue = mnist[i * cols + j]
                                  - mnist[I * cols + j];
                        acc += residue * residue;
                    }

                    // write Delta[i,i'] = Delta[i',i]
                    all_pairs[i * rows + I] = acc;
                    all_pairs[I * rows + i] = acc;
                }
            }
        }
    };

    std::vector<std::thread> threads;

    for (size_t id = 0; id < num_threads; ++id)
        threads.emplace_back(dynamic_block_cyclic);

    for (auto& thread : threads)
        thread.join();
}

template<class T>
void dynamic_all_pairs_rev(const T* mnist,
                           T* all_pairs,
                           size_t rows,
                           size_t cols,
                           size_t num_threads = 64,
                           size_t chunk_size  = 64 / sizeof(T)) {
    size_t global_lower = 0;
    std::mutex mutex;

    auto dynamic_block_cyclic = [&]() {
        // assume we have not done anything
        size_t lower = 0;

        // while there are still rows to compute
        while (lower < rows) {
            // update lower row with global lower row
            {
                std::scoped_lock lock(mutex);
                lower = global_lower;
                global_lower += chunk_size;
            }

            // compute the upper border of the block (exclusive)
            const size_t upper = rows >= lower ? rows - lower : 0;
            const size_t LOWER = upper >= chunk_size ? upper - chunk_size : 0;

            // for all entries below the diagonal (i'=I)
            for (size_t i = LOWER; i < upper; ++i) {
                for (size_t I = 0; I <= i; ++I) {
                    // compute squared Euclidean distance
                    T acc = T(0);
                    for (size_t j = 0; j < cols; ++j) {
                        T residue = mnist[i * cols + j] - mnist[I * cols + j];
                        acc += residue * residue;
                    }

                    // write Delta[i,i'] = Delta[i',i]
                    all_pairs[i * rows + I] = all_pairs[I * rows + i] = acc;
                }
            }
        }
    };

    std::vector<std::thread> threads;

    for (size_t id = 0; id < num_threads; ++id)
        threads.emplace_back(dynamic_block_cyclic);

    for (auto& thread : threads)
        thread.join();
}

int main(int argc, char** argv) {
    if (argc != 4) return EXIT_FAILURE;
    const char* file         = argv[1];
    const size_t num_threads = std::stoi(argv[2]);
    const size_t distr       = std::stoi(argv[3]);
    const size_t rows        = 65000;
    const size_t cols        = 28 * 28;

    // load MNIST data from binary file
    auto tl          = start_timer();
    const auto mnist = std::make_unique<float[]>(rows * cols);
    load_binary(mnist.get(), rows * cols, file);
    stop_timer(tl, "load_data_from_disk");

    auto tc       = start_timer();
    auto all_pairs = std::make_unique<float[]>(rows * rows);
    switch (distr) {
        case 0: sequential_all_pairs (mnist.get(), all_pairs.get(), rows, cols);              break;
        case 1: parallel_all_pairs   (mnist.get(), all_pairs.get(), rows, cols, num_threads); break;
        case 2: dynamic_all_pairs    (mnist.get(), all_pairs.get(), rows, cols, num_threads); break;
        case 3: dynamic_all_pairs_rev(mnist.get(), all_pairs.get(), rows, cols, num_threads); break;
        default: return EXIT_FAILURE;
    }
    stop_timer(tc, "compute_distances");

    auto td = start_timer();
    dump_binary(all_pairs.get(), rows * rows, "./all_pairs.bin");
    stop_timer(td, "dump_to_disk");
}
