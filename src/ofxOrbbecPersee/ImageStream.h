#pragma once

// stdlib
#include <memory>
// OF
#include "ofMain.h"

namespace ofxOrbbecPersee {

  class ImageStream;
  typedef std::shared_ptr<ImageStream> ImageStreamRef ;

  class ImageStream {
    public: // types & consts

      struct Options {

      };

    public: // methods
      ImageStream() : pixFront(&pix1), pixBack(&pix2){}

      void setup(int width, int height, ofImageType fmt);
      void destroy();

      const ofTexture& getTexture() const { return tex; }

    protected:

      void swap() {
        std::swap(this->pixFront, this->pixBack);
      }

    protected: // attributes
      ofTexture tex;
      ofPixels pix1, pix2, *pixFront, *pixBack;
  };
}
