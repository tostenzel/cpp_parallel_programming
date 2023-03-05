// ./mandelbrot 256 1920 1200 -2.0 1.0 -1.0 1.0 out.ppm

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <thread>
#include <tuple>
#include <vector>

template <class T> auto ceil_div(T x, T y) { return (x + y - T(1)) / y; }

using namespace std;

class MandelBrot {
  double x0;
  double x1;
  double xDist;
  double y0;
  double y1;
  double yDist;

  size_t max_iters;
  size_t width;
  size_t height;

  vector<char> matrix;

public:
  MandelBrot(double x0, double x1, double y0, double y1, size_t max_iters,
             size_t width, size_t height) {
    this->matrix = vector<char>(width * height);

    this->x0 = x0;
    this->x1 = x1;
    this->xDist = x1 - x0;
    this->y0 = y0;

    this->y1 = y1;
    this->yDist = y1 - y0;
    this->max_iters = max_iters;
    this->width = width;
    this->height = height;
  }

  tuple<double, double> mapIndexToComplex(int index) {
    int x = index % this->width;
    int y = index / this->width;
    return mapPixelToComplex(x, y);
  }

  tuple<double, double> mapPixelToComplex(int x, int y) {
    double cx = ((double)x / (double)this->width) * this->xDist + this->x0;
    double cy = ((double)y / (double)this->height) * this->yDist + this->y0;
    return make_tuple(cx, cy);
  }

  char calculateMandelBrotPixel(int xPixel, int yPixel) {
    double xRoot = 0.0, yRoot = 0.0, xCurrent = 0.0, yCurrent = 0.0,
           xTemp = 0.0;
    tie(xRoot, yRoot) = mapPixelToComplex(xPixel, yPixel);

    unsigned int iteration = 0;
    while (xCurrent * xCurrent + yCurrent * yCurrent <= 2.0 * 2.0 &&
           iteration < this->max_iters) {
      xTemp = xCurrent * xCurrent - yCurrent * yCurrent + xRoot;
      yCurrent = 2.0 * xCurrent * yCurrent + yRoot;
      xCurrent = xTemp;
      iteration++;
    }

    char color = (iteration / (float)this->max_iters) * 255;
    return color;
  }

  void calculateMandelBrotRangeIntrinsic(size_t fromIndex, size_t toIndex) {
    // Define needed in memory variables
    double xRootsMem[4] = {0, 0, 0, 0};
    double yRootsMem[4] = {0, 0, 0, 0};
    double xCurrentsMem[4] = {0, 0, 0, 0};
    double yCurrentsMem[4] = {0, 0, 0, 0};
    double xCurrent2PlusYCurrent2Mem[4] = {0, 0, 0, 0};
    unsigned int iterationsMem[4] = {0, 0, 0, 0};
    size_t workIndices[4] = {0, 0, 0, 0};
    size_t workIndex = fromIndex + 4;
    bool registersDirty[4] = {false, false, false, false};
    bool dirty = false;

    // Calculate original x and yRoots
    for (int packedIndex = 0; packedIndex < 4; packedIndex++) {
      tie(xRootsMem[packedIndex], yRootsMem[packedIndex]) =
          mapIndexToComplex(fromIndex + packedIndex);
      workIndices[packedIndex] = fromIndex + packedIndex;
    }

    // Prepare SIMD registers
    __m256d xRoots = _mm256_loadu_pd(xRootsMem);
    __m256d yRoots = _mm256_loadu_pd(yRootsMem);
    __m256d xCurrents = _mm256_setzero_pd();
    __m256d yCurrents = _mm256_setzero_pd();
    __m256d xCurrents2 = _mm256_setzero_pd();
    __m256d yCurrents2 = _mm256_setzero_pd();
    __m256d xCurrent2PlusYCurrent2 = _mm256_setzero_pd();
    __m256d xCurrent2MinusYCurrent2 = _mm256_setzero_pd();
    __m256d xTemps = _mm256_setzero_pd();
    __m256d constants2 = _mm256_set1_pd(2.0);

    while (true) {
      // Store calculation to memory
      _mm256_storeu_pd(xCurrent2PlusYCurrent2Mem, xCurrent2PlusYCurrent2);

      // Check per work / SIMD registry part stopping rule
      // while (xCurrent * xCurrent + yCurrent * yCurrent <= 4.0 && iteration <
      // this->max_iters)
      for (int packedIndex = 0; packedIndex < 4; packedIndex++) {
        if (!(xCurrent2PlusYCurrent2Mem[packedIndex] <= 4.0 &&
              iterationsMem[packedIndex] < this->max_iters)) {
          // Save work to memory
          char color =
              (iterationsMem[packedIndex] / (float)this->max_iters) * 255;
          size_t matrix_index = workIndices[packedIndex];
          this->matrix[matrix_index] = color;

          // Assign new work
          // Assign new work index
          workIndex++;
          if (workIndex <= toIndex) {
            workIndices[packedIndex] = workIndex;
          } else {
            workIndices[packedIndex] = toIndex;
            workIndex--;
          }

          // Assign new xRoot and yRoot
          tie(xRootsMem[packedIndex], yRootsMem[packedIndex]) =
              mapIndexToComplex(workIndex);

          // Reset iteration counter
          iterationsMem[packedIndex] = 0;

          // Mark registers to be dirty
          registersDirty[packedIndex] = true;
          dirty = true;

          // If everyone is now calculating toIndex, halt
          if (workIndices[0] == toIndex && workIndices[1] == toIndex &&
              workIndices[2] == toIndex && workIndices[3] == toIndex) {
            return;
          }
        }
      }

      // Update registers if they are dirty
      if (dirty) {
        dirty = false;

        // Update xRoot and yRoot
        xRoots = _mm256_loadu_pd(xRootsMem);
        yRoots = _mm256_loadu_pd(yRootsMem);

        // Update xCurrent and yCurrent
        _mm256_storeu_pd(xCurrentsMem, xCurrents);
        _mm256_storeu_pd(yCurrentsMem, yCurrents);
        for (int packedIndex = 0; packedIndex < 4; packedIndex++) {
          // If dirty zero xCurrent and yCurrent
          if (registersDirty[packedIndex]) {
            xCurrentsMem[packedIndex] = 0;
            yCurrentsMem[packedIndex] = 0;
          }
          // Clear dirty flag
          registersDirty[packedIndex] = false;
        }
        xCurrents = _mm256_loadu_pd(xCurrentsMem);
        yCurrents = _mm256_loadu_pd(yCurrentsMem);
      }

      // Calculate xCurrent * xCurrent + yCurrent * yCurrent
      xCurrents2 = _mm256_mul_pd(xCurrents, xCurrents);
      yCurrents2 = _mm256_mul_pd(yCurrents, yCurrents);
      xCurrent2PlusYCurrent2 = _mm256_add_pd(xCurrents2, yCurrents2);

      // Calculate xCurrent * xCurrent - yCurrent * yCurrent
      xCurrent2MinusYCurrent2 = _mm256_sub_pd(xCurrents2, yCurrents2);

      // Calculate xCurrent * xCurrent - yCurrent * yCurrent + xRoot
      xTemps = _mm256_add_pd(xCurrent2MinusYCurrent2, xRoots);

      // Calculate new yCurrent 2.0 * xCurrent * yCurrent + yRoot
      yCurrents = _mm256_mul_pd(xCurrents, yCurrents);
      yCurrents = _mm256_mul_pd(constants2, yCurrents);
      yCurrents = _mm256_add_pd(yCurrents, yRoots);

      // Update xCurrent
      xCurrents = xTemps;

      // Update iteration counters
      for (int packedIndex = 0; packedIndex < 4; packedIndex++) {
        iterationsMem[packedIndex]++;
      }
    }
  }

  void calculateMandelBrotSequentially() {
    for (unsigned int y = 0; y < this->height; y++) {
      for (unsigned int x = 0; x < this->width; x++) {
        this->matrix[y * width + x] = calculateMandelBrotPixel(x, y);
      }
    }
  }

  void calculateMandelBrotSequentiallyIntrinsic() {
    calculateMandelBrotRangeIntrinsic(0, this->height * this->width - 1);
  }

  void calculateMandelBrotParallel() {
    size_t vectorLength = this->width * this->height;
    const auto processor = std::thread::hardware_concurrency();
    size_t num_threads = (processor != 0) ? processor * 2 : 4;

    auto block = [=](size_t threadIndex) {
      size_t chunk = ceil_div(vectorLength, num_threads);
      size_t lower = threadIndex * chunk;
      size_t upper = std::min(lower + chunk, vectorLength);

      for (size_t i = lower; i < upper; i++) {
        unsigned int x = i % this->width;
        unsigned int y = i / width;
        this->matrix[i] = calculateMandelBrotPixel(x, y);
      }
    };

    std::vector<std::thread> rand_threads;
    for (size_t i = 0; i < num_threads; i++)
      rand_threads.emplace_back(block, i);
    for (auto &thread : rand_threads)
      thread.join();
  }

  void calculateMandelBrotParallelIntrinsic() {
    size_t vectorLength = this->width * this->height;
    const auto processor = std::thread::hardware_concurrency();
    size_t num_threads = (processor != 0) ? processor * 2 : 4;

    auto block = [=](size_t threadIndex) {
      size_t chunk = ceil_div(vectorLength, num_threads);
      size_t lower = threadIndex * chunk;
      size_t upper = std::min(lower + chunk, vectorLength);

      calculateMandelBrotRangeIntrinsic(lower, upper - 1);
    };

    std::vector<std::thread> rand_threads;
    for (size_t i = 0; i < num_threads; i++)
      rand_threads.emplace_back(block, i);
    for (auto &thread : rand_threads)
      thread.join();
  }

  void saveMandelBrot(string fileName) {
    ofstream ofs(fileName, ios_base::out | ios_base::binary);
    ofs << "P6" << endl
        << this->width << ' ' << this->height << endl
        << "255" << endl;

    for (auto y = 0u; y < this->height; ++y) {
      for (auto x = 0u; x < this->width; ++x) {
        char color = this->matrix[y * width + x];
        ofs << color << color << color; // red, green, blue
      }
    }

    ofs.close();
  }
};

int main(int argc, char **argv) {
  if (argc != 9) {
    cerr << "usage:" << argv[0]
         << " <num_iters> <width> <height> <x0> <x1> <y0> <y1>" << endl;
    return EXIT_FAILURE;
  }

  size_t max_iters = stoi(argv[1]);
  size_t width = stoi(argv[2]);
  size_t height = stoi(argv[3]);
  float x0 = stof(argv[4]);
  float x1 = stof(argv[5]);
  float y0 = stof(argv[6]);
  float y1 = stof(argv[7]);
  const char *filename = argv[8];

  MandelBrot mandelBrot = MandelBrot(x0, x1, y0, y1, max_iters, width, height);
  // mandelBrot.calculateMandelBrotSequentially();
  mandelBrot.calculateMandelBrotParallelIntrinsic();
  mandelBrot.saveMandelBrot(filename);
}