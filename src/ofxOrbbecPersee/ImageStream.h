#pragma once

// stdlib
#include <memory>
// OF
#include "ofMain.h"
// local
#include "persee/Receiver.h"
#include "persee/Inflater.h"

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

      persee::InflaterRef getInflater() {
        if(!this->inflaterRef)
          this->inflaterRef = std::make_shared<persee::Inflater>();
        return this->inflaterRef;
      }

    protected:

      void swap() {
        std::swap(this->pixFront, this->pixBack);
      }

    protected: // attributes
      ofTexture tex;
      ofPixels pix1, pix2, *pixFront, *pixBack;
      persee::ReceiverRef receiverRef=nullptr;
      persee::InflaterRef inflaterRef=nullptr;
  };
}
