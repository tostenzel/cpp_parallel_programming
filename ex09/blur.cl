float sgm[7][7] = {
    {0.00134f, 0.00408f, 0.00794f, 0.00992f, 0.00794f, 0.00408f, 0.00134f},
    {0.00408f, 0.01238f, 0.02412f, 0.03012f, 0.02412f, 0.01238f, 0.00408f},
    {0.00794f, 0.02412f, 0.04698f, 0.05867f, 0.04698f, 0.02412f, 0.00794f},
    {0.00992f, 0.03012f, 0.05867f, 0.07327f, 0.05867f, 0.03012f, 0.00992f},
    {0.00794f, 0.02412f, 0.04698f, 0.05867f, 0.04698f, 0.02412f, 0.00794f},
    {0.00408f, 0.01238f, 0.02412f, 0.03012f, 0.02412f, 0.01238f, 0.00408f},
    {0.00134f, 0.00408f, 0.00794f, 0.00992f, 0.00794f, 0.00408f, 0.00134f}};

float identity[7][7] = {{0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                        {0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

int getXByIndex(int index, int width) { return (index / 3) % width; }

int getYByIndex(int index, int width) { return (index / 3) / width; }

int getIndexByXY(int x, int y, int width) { return y * width * 3 + x * 3; }

// you can change this as you like
__kernel void blur(__global float *in, __global float *out, int width,
                   int height) {
  const int index = get_global_id(0) * 3;

  // Get current coordinates
  int currentX = getXByIndex(index, width);
  int currentY = getYByIndex(index, width);

  // Check if this is a boundary pixel
  if (currentX < 3 || currentX > width - 3 || currentY < 3 ||
      currentY > height - 3) {
    // Copy input to output
    out[index] = in[index];
    out[index + 1] = in[index + 1];
    out[index + 2] = in[index + 2];
  }

  float accumulator[3] = {0.f, 0.f, 0.f};
  for (int samplingX = -3; samplingX <= 3; samplingX++) {
    for (int samplingY = -3; samplingY <= 3; samplingY++) {
      float gaussianValue = sgm[samplingY + 3][samplingX + 3];
      int sampleIndex =
          getIndexByXY(currentX + samplingX, currentY + samplingY, width);
      for (int colorChanelIndex = 0; colorChanelIndex < 3; colorChanelIndex++) {
        float sampleValue = in[sampleIndex + colorChanelIndex];
        accumulator[colorChanelIndex] += gaussianValue * sampleValue;
      }
    }
  }
  out[index] = accumulator[0];
  out[index + 1] = accumulator[1];
  out[index + 2] = accumulator[2];
}