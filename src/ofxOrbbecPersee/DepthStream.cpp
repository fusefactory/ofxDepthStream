#include "DepthStream.h"

using namespace ofxOrbbecPersee;

void DepthStream::setup(int width, int height) {
  ImageStream::setup(width, height, OF_IMAGE_GRAYSCALE);
}

void DepthStream::update() {
  if(receiverRef && receiverRef->hasNew()) {
    auto inflater = this->getInflater();
    if(inflater->inflate(receiverRef->getFrameData(), receiverRef->getSize())) {
      this->updatePixels(inflater->getData(), inflater->getSize());
    }
  }
}

void DepthStream::updatePixels(const void* data, size_t size) {
  // TODO check is size matches with allocated buffer size?!
  pixBack->setFromPixels((const unsigned char *)data, pixBack->getWidth(), pixBack->getHeight(), OF_IMAGE_GRAYSCALE);

  // should the following happen here?
  tex.loadData(*pixBack);
  this->swap();
}
