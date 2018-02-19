#include "ImageStream.h"

using namespace ofxOrbbecPersee;

void ImageStream::setup(int width, int height, ofImageType fmt) {
  this->destroy();
  pix1.allocate(width, height, fmt);
  pix2.allocate(width, height, fmt);
  tex.allocate(pix1);
}

void ImageStream::destroy() {
  tex.clear();
  pix1.clear();
  pix2.clear();
}
