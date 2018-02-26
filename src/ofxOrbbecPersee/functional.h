#pragma once

// OF
#include "ofMain.h"
// local/persee
#include "persee.h"

namespace ofxOrbbecPersee {

  /**
   * These receiver-to-texture update methods are convenience methods, meant
   * mostly as quick-start for demo purposes. In production they can be optimized
   * by caching the used ofPixels and Inflator instances.
   **/
  void receiverToGrayscaleTexture(persee::Receiver& receiver, ofTexture& depthTex) {
    // check if buffer has data
    persee::emptyBuffer(receiver, [&depthTex](const void* data, size_t size){

      ofPixels depthPixels;

      // allocate if necessary
      if(depthTex.isAllocated()) {
        depthPixels.allocate(depthTex.getWidth(), depthTex.getHeight(), OF_IMAGE_GRAYSCALE);
      } else {
        // TODO; infer texture size from receiver frame size?
        depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
        depthTex.allocate(depthPixels);
      }

      // returns shared_ptr<persee::Frame> with inflated data
      auto inflatedFrameRef = persee::inflate(data, size);

      if(inflatedFrameRef) {
        // returns shared_ptr<persee::Frame> with 1-byte grayscale data
        inflatedFrameRef->convert(persee::grayscale255bitConverter(depthPixels.getWidth(), depthPixels.getHeight()))
        // load grayscale data into our ofTexture instance
        ->convert<void>([&depthPixels, &depthTex](const void* data, size_t size){
          // ofLogNotice() << "buffer to tex onversion update: " << size;
          depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
          depthTex.loadData(depthPixels);
        });
      }
    });
  }

  /**
   * These receiver-to-texture update methods are convenience methods, meant
   * mostly as quick-start for demo purposes. In production they can be optimized
   * by caching the used ofPixels and Inflator instances.
   **/
  void receiverToColorTexture(persee::Receiver& receiver, ofTexture& texture) {
    // check if buffer has data
    persee::emptyBuffer(receiver, [&texture](const void* data, size_t size){

      ofPixels pixels;

      // allocate if necessary
      if(texture.isAllocated()) {
        pixels.allocate(texture.getWidth(), texture.getHeight(), OF_IMAGE_COLOR);
      } else {
        // TODO; infer texture size from receiver frame size?
        pixels.allocate(1280, 720, OF_IMAGE_COLOR);
        texture.allocate(pixels);
      }

      // returns shared_ptr<persee::Frame> with inflated data
      auto inflatedFrameRef = persee::inflate(data, size);

      if(inflatedFrameRef) {
        // returns shared_ptr<persee::Frame> with 1-byte grayscale data
        // ->convert(persee::grayscale255bitConverter(pixels.getWidth(), pixels.getHeight()))
        // load grayscale data into our ofTexture instance
        inflatedFrameRef->convert<void>([&pixels, &texture](const void* data, size_t size){
          // ofLogNotice() << "buffer to tex onversion update: " << size;
          pixels.setFromPixels((const unsigned char *)data, pixels.getWidth(), pixels.getHeight(), OF_IMAGE_GRAYSCALE);
          texture.loadData(pixels);
        });
      }
    });
  }
}
