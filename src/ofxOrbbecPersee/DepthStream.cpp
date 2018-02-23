#include "DepthStream.h"

using namespace ofxOrbbecPersee;

void DepthStream::setup(int width, int height) {
  ImageStream::setup(width, height, OF_IMAGE_GRAYSCALE);
}

void DepthStream::updatePixels(const void* data, size_t size) {
  size_t texSize = pixBack->getWidth() * pixBack->getHeight() * 1;
  unsigned char converted[texSize];


  for(int i=0; i<texSize; i++) {
    // unsigned char a = ((const unsigned char*)data)[i*2+1];
    unsigned char b = ((const unsigned char*)data)[i*2];
    converted[i] = b; //(unsigned char)(((a << 8) | b) >> 8);//(unsigned char)val;
  }

  // ofLogNotice() << "last value:" << (int)converted[texSize-1];

  // TODO check is size matches with allocated buffer size?!
  pixBack->setFromPixels((const unsigned char *)converted, pixBack->getWidth(), pixBack->getHeight(), OF_IMAGE_GRAYSCALE);

  // should the following happen here?
  tex.loadData(*pixBack);
  this->swap();
}
