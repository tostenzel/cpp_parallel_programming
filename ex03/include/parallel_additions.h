#include <iostream>
#include <memory>
#include <thread>
#include <vector>

template <class T> auto ceil_div(T x, T y) { return (x + y - T(1)) / y; }

template <class T>
void additionBlockDist(const T *vector1, const T *vector2, T *output,
                       size_t vectorLength, size_t numThreads) {
  auto block = [=](size_t threadIndex) {
    size_t chunk = ceil_div(vectorLength, numThreads);
    size_t lower = threadIndex * chunk;
    size_t upper = std::min(lower + chunk, vectorLength);
    for (size_t i = lower; i < upper; i++) {
      (*output)[i] = (*vector1)[i] + (*vector2)[i];
    }
  };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < numThreads; i++)
    threads.emplace_back(block, i);
  for (auto &thread : threads)
    thread.join();
}

template <class T>
void additionCyclicDist(const T *vector1, const T *vector2, T *output,
                        size_t vector_length, size_t numThreads) {
  auto cyclic = [=](size_t threadIndex) {
    for (size_t i = threadIndex; i < vector_length; i += numThreads) {
      (*output)[i] = (*vector1)[i] + (*vector2)[i];
    }
  };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < numThreads; i++)
    threads.emplace_back(cyclic, i);
  for (auto &thread : threads)
    thread.join();
}

template <class T>
void additionBlockCyclicDist(const T *vector1, const T *vector2, T *output,
                             size_t vector_length, size_t blockSize,
                             size_t numThreads) {
  auto block = [=](size_t threadIndex) {
    for (size_t block = threadIndex * blockSize; block < vector_length;
         block += blockSize) {
      for (size_t blockIndex = 0; blockIndex < blockSize; blockIndex++) {
        size_t i = block + blockIndex;
        (*output)[i] = (*vector1)[i] + (*vector2)[i];
      }
    }
  };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < numThreads; i++)
    threads.emplace_back(block, i);
  for (auto &thread : threads)
    thread.join();
}