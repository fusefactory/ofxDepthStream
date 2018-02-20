#include <iostream>
#include <math.h>
#include "Formatter.h"

using namespace openni;
using namespace persee;

#ifdef OPENNI_AVAILABLE
void Formatter::process(VideoStream& stream) {
  VideoFrameRef frame;
  stream.readFrame(&frame);

  int pixelCount = frame.getHeight()*frame.getWidth();
  size_t pixelSize;
  switch (frame.getVideoMode().getPixelFormat())
  {
    case PIXEL_FORMAT_DEPTH_1_MM:
    case PIXEL_FORMAT_DEPTH_100_UM: {
      pixelSize = sizeof(DepthPixel);
      this->size = pixelCount * pixelSize;
      this->data = (const char*)frame.getData();
      // memcpy(buffer, frame.getData(), this->size);
      // std::cout << "formatter: found " << this->size << " depth bytes" << std::endl;
      break;
    }

    case PIXEL_FORMAT_RGB888: {
      pixelSize = sizeof(RGB888Pixel);
      this->size = pixelCount * pixelSize;
      this->data = (const char*)frame.getData();
      // memcpy(buffer, frame.getData(), this->size);
      // std::cout << "formatter: found " << this->size << " color bytes" << std::endl;
      break;
    }

    default:
      std::cerr << "unsupported pixel format" << std::endl;
  }
}

#else

// Dummy implementation which provides a
void Formatter::process(VideoStream& stream) {
  this->size = BUF_SIZE;
  for(int i=0; i<size; i+=2){
    int v = (sinf(i*0.1f) + 1.0f) / 2.0f * ((1 << 15)-1);

    this->data[i+1] = (v & 0xff) << 8;
    this->data[i] = (v & 0xff);
  }
}

#endif
