#pragma once

#include <memory>
#include "ImageStream.h"

namespace ofxOrbbecPersee {
  class ColorStream;
  typedef std::shared_ptr<ColorStream> ColorStreamRef;

  class ColorStream : public ImageStream {
    public: // types and consts
      static const int FRAME_SIZE_1280x720x24BIT = (1280*720*3);

    public:
      void setup(){ this->setup(1280, 720); };
      void setup(int width, int height);

    protected:
      virtual void updatePixels(const void* data, size_t size);
  };
}
