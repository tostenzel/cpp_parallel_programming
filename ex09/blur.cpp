#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

// PoCL only supports OpenCL 1.2, though
#define CL_HPP_TARGET_OPENCL_VERSION 200

// These are the C++ bindings available here:
// https://github.com/KhronosGroup/OpenCL-CLHPP
// But you can also use the plain C-API, if you wish.
#include <CL/opencl.hpp>

using namespace std;

class Blur {
public:
  int width;
  int height;


  float *processImage(float *originalImage, const char *openCLProgramPath) {
    // Prepare OpenCL program
    cl::Context context(CL_DEVICE_TYPE_DEFAULT);
    cl::CommandQueue queue(context);

    // Create buffers
    int memorySize = width * height * 3;
    cl::Buffer in = cl::Buffer(queue, originalImage, originalImage + memorySize,
                               true, true, NULL);
    cl::Buffer out =
        cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * memorySize);

    // Compile kernel
    cl::Program blurProg = load(context, openCLProgramPath);
    cl::Kernel blurKernel = cl::Kernel(blurProg, "blur", NULL);

    // Set arguments for the kernel
    cl_int clWidth = (cl_int)width;
    cl_int clHeight = (cl_int)height;
    blurKernel.setArg(0, in);
    blurKernel.setArg(1, out);
    blurKernel.setArg(2, clWidth);
    blurKernel.setArg(3, clHeight);

    // Execute kernel
    queue.enqueueNDRangeKernel(blurKernel, cl::NullRange,
                               cl::NDRange(width * height));

    // Results back to memory
    float *outLocal = new float[memorySize];
    cl::copy(queue, out, outLocal, outLocal + memorySize);
    return outLocal;
  }

  // By prof. Leissa
  cl::Program load(cl::Context context, const char *filename) {
    if (auto stream = ifstream(filename)) {
      string str(istreambuf_iterator<char>(stream),
                 (istreambuf_iterator<char>()));
      cl_int err;
      cl::Program prog(context, str, true, &err);
      if (err == CL_BUILD_PROGRAM_FAILURE) {
        auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
        auto log = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        cerr << "Build Log: " << log << endl;
        exit(1);
      }
      return prog;
    }
    cerr << "error: cannot open file: " << filename << endl;
    exit(1);
  }

  void writePPM(string fileName, float *blurredImage) {
    // ppm write image file
    ofstream outFile(fileName, ios_base::out | ios_base::binary);
    outFile << "P6" << endl << width << ' ' << height << endl << "255" << endl;

    size_t size = height * width * 3;
    for (size_t idx = 0; idx < size; idx++) {
      outFile << char(blurredImage[idx] * 255); // red, green, blue
    }

    outFile.close();
    //cout << "Image Printed" << endl;
  }


  float *readPPM(string fileName) {
    // ppm read image file
    ifstream inputFile(fileName, ios_base::in | ios_base::binary);

    auto readNoneComment = [&]  () -> string {
      string noneComment;
      getline(inputFile, noneComment);
      while (noneComment[0] == '#') {
              getline(inputFile, noneComment);

          }
      return noneComment;
    };

    // Read out file header
    string format = readNoneComment();

    // Read out width and height
    string heightString, widthString;
    string dims = readNoneComment();
    stringstream dimensions(dims);
    //dimensions << dims;

    try {
      dimensions >> width;
      dimensions >> height;
        } catch (std::exception &e) {

        }

    cout << width << endl;
    cout << height << endl;

    string brightness = readNoneComment();

    float *image = new float[width * height * 3];
    char byte;
    for (int index = 0; index < width * height * 3; index++) {
      inputFile.get(byte);
      image[index] = (float)byte / (float)255;
    }

    inputFile.close();

    //cout << "READ IMAGE" << endl;
    return image;
  }
};

int main(int argc, char **argv) {
  if (argc != 4)
    return EXIT_FAILURE;
  auto input = argv[1];
  auto output = argv[2];
  auto cl = argv[3];

  Blur blur = Blur();
  float *inputImage = blur.readPPM(input);
  float *outputImage = blur.processImage(inputImage, cl);
  blur.writePPM(output, outputImage);
  free(inputImage); // very important
  free(outputImage);

  return EXIT_SUCCESS;
}

//./blur <input.ppm> <output.ppm> blur.cl