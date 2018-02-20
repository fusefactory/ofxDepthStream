#pragma once

#include <memory>
#include "ImageStream.h"

namespace ofxOrbbecPersee {
  class DepthStream;
  typedef std::shared_ptr<DepthStream> DepthStreamRef;

  class DepthStream : public ImageStream {
    public: // types and consts
      static const int FRAME_SIZE_640x480x16BIT = (640*480*2);

    public:

      void setup(){ this->setup(640, 480); };
      void setup(int width, int height);
      void update() override;

    protected:
      void updatePixels(const void* data, size_t size);
  };
}
