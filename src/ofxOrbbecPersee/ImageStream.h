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

      class Addon {
      public:
        Addon(ImageStream* imagestream) : imgstream(imagestream){}
        ~Addon(){ destroy(); }
        virtual void update(){};

      protected:
        virtual void setup(){}
        virtual void destroy(){};

        ImageStream* getImageStream(){ return imgstream; }
      private:
        ImageStream* imgstream;
      };

    public: // methods
      ImageStream() : pixFront(&pix1), pixBack(&pix2){}
      ~ImageStream() { this->destroy(); }

      void setup(int width, int height, ofImageType fmt);
      virtual void update();

      void destroy();
      const ofTexture& getTexture() const { return tex; }

      void offerData(void* data, size_t size) {
        offeredData = data;
        offeredSize = size;
      }

      void addAddon(std::shared_ptr<Addon> newaddon) {
        addons.push_back(newaddon);
      }

    protected:

      virtual void updatePixels(const void* data, size_t size){};

      void swap() {
        std::swap(this->pixFront, this->pixBack);
      }

    protected: // attributes
      ofTexture tex;
      ofPixels pix1, pix2, *pixFront, *pixBack;

      void* offeredData=NULL;
      size_t offeredSize=0;
      std::vector<std::shared_ptr<Addon>> addons;
  };
}
