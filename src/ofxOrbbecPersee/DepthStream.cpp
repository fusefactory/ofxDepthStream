#include "DepthStream.h"

using namespace ofxOrbbecPersee;

void DepthStream::setup(int width, int height) {
  ImageStream::setup(width, height, OF_IMAGE_GRAYSCALE);
}

void DepthStream::updatePixels(const void* data, size_t size) {
  size_t half = size >> 1;
  unsigned char converted[half];

  for(int i=0; i<half; i++) {
    converted[i] = ((
      ((const unsigned char*)data)[i*2] + ((const unsigned char*)data)[i*2+1]) >> 1);
  }

  // TODO check is size matches with allocated buffer size?!
  pixBack->setFromPixels((const unsigned char *)converted, pixBack->getWidth(), pixBack->getHeight(), OF_IMAGE_GRAYSCALE);

  // should the following happen here?
  tex.loadData(*pixBack);
  this->swap();
}
