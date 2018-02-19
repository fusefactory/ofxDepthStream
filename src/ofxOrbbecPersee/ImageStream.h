#pragma once

// stdlib
#include <memory>
// OF
#include "ofMain.h"
// local
#include "persee/Receiver.h"

namespace ofxOrbbecPersee {

  class ImageStream;
  typedef std::shared_ptr<ImageStream> ImageStreamRef ;

  class ImageStream {
    public: // types & consts

      struct Options {

      };

    public: // methods
      ImageStream() : pixFront(&pix1), pixBack(&pix2){}
      ~ImageStream() { this->destroy(); }

      void setup(int width, int height, ofImageType fmt);
      virtual void update() {};
      void destroy();

      const ofTexture& getTexture() const { return tex; }

      void setReceiver(persee::ReceiverRef recvr) { this->receiverRef = recvr; }

    protected:

      void swap() {
        std::swap(this->pixFront, this->pixBack);
      }

    protected: // attributes
      ofTexture tex;
      ofPixels pix1, pix2, *pixFront, *pixBack;
      persee::ReceiverRef receiverRef=nullptr;
  };
}
