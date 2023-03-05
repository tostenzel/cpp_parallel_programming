#include <chrono>
#include <random>
#include <sstream>
#include <vector>

#include "parallel_additions.h"

template <class T> class Matrix {
  std::vector<T> m;
  unsigned int dimX;
  unsigned int dimY;
  unsigned int parallelizationMode_ = 0;
  unsigned int numThreads_ = 1;
  unsigned int blockSize_ = 16;
  int64_t lastExecutionTime;

public:
  Matrix(unsigned int dimX, unsigned int dimY) {
    this->dimX = dimX;
    this->dimY = dimY;
    m.resize(dimX * dimY);
  }

  class MatrixRow {
    std::vector<T> *m;
    unsigned int *dimY;
    unsigned int x;

  public:
    MatrixRow(std::vector<T> *m, unsigned int *dimY, unsigned int x) {
      this->m = m;
      this->dimY = dimY;
      this->x = x;
    }
    T &operator[](unsigned int y) { return m->at(x + y * (*dimY)); }
  };

  unsigned int getDimX() { return this->dimX; }

  unsigned int getDimY() { return this->dimY; }

  int64_t getLastExecutionTime() { return this->lastExecutionTime; }

  void setParallelizationMode(unsigned int newParallelizationMode) {
    this->parallelizationMode_ = newParallelizationMode;
  }

  void setNumThreads(unsigned int newNumThreads) {
    this->numThreads_ = newNumThreads;
  }

  void setBlockSize(unsigned int newBlockSize) {
    this->blockSize_ = newBlockSize;
  }

  void randomizeMatrix() {

    size_t vectorLength = this->dimX * this->dimY;
    size_t num_threads = this->numThreads_;

    auto block = [=](size_t threadIndex) {
      size_t chunk = ceil_div(vectorLength, num_threads);
      size_t lower = threadIndex * chunk;
      size_t upper = std::min(lower + chunk, vectorLength);

      size_t seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(lower + seed);
      std::uniform_int_distribution<int> distribution(0, 100);

      for (size_t i = lower; i < upper; i++) {
        (this->m)[i] = distribution(generator);
        ;
      }
    };

    std::vector<std::thread> rand_threads;
    for (size_t i = 0; i < num_threads; i++)
      rand_threads.emplace_back(block, i);
    for (auto &thread : rand_threads)
      thread.join();
  }

  MatrixRow operator[](unsigned int x) {
    MatrixRow matrixRow = MatrixRow(&m, &dimY, x);
    return matrixRow;
  }

  Matrix operator+(const Matrix &matrix2) {
    Matrix output = Matrix(this->dimX, this->dimY);

    // Start timer
    auto timer1 = std::chrono::system_clock::now();

    switch (this->parallelizationMode_) {
    case 0:
      additionBlockDist(&this->m, &(matrix2.m), &(output.m),
                        this->dimX * this->dimY, this->numThreads_);
      break;
    case 1:
      additionCyclicDist(&(this->m), &(matrix2.m), &(output.m),
                         this->dimX * this->dimY, this->numThreads_);
      break;
    case 2:
      additionBlockCyclicDist(&(this->m), &(matrix2.m), &(output.m),
                              this->dimX * this->dimY, this->blockSize_,
                              this->numThreads_);
      break;
    }

    // Stop timer
    auto timer2 = std::chrono::system_clock::now();
    this->lastExecutionTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(timer2 - timer1)
            .count();

    return output;
  }

  // Basically print operation override
  friend std::ostream &operator<<(std::ostream &os, const Matrix<T> &matrix) {
    std::stringstream output;
    for (size_t y = 0; y < matrix.dimY; y++) {
      for (size_t x = 0; x < matrix.dimX; x++) {
        output << matrix.m.at(x + y * (matrix.dimY)) << ", ";
      }
      output << "\n";
    }
    return os << output.str();
  }
};