#pragma once

// OF
#include "ofMain.h"
// local/persee
#include "persee.h"

namespace ofxOrbbecPersee {

  void loadGrayscaleTexture(ofTexture& depthTex, const void* data, size_t size) {
    ofPixels depthPixels;

    // allocate
    if(depthTex.isAllocated()) {
      depthPixels.allocate(depthTex.getWidth(), depthTex.getHeight(), OF_IMAGE_GRAYSCALE);
    } else {
      // TODO; infer texture size from receiver frame size?
      depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
      depthTex.allocate(depthPixels);
    }

    // check
    if( (depthTex.getWidth() * depthTex.getHeight()) * 2 != size ) {
      ofLogWarning() << "Color texture size did not match data-size";
      return;
    }

    // returns shared_ptr<persee::Frame> with 1-byte grayscale data
    persee::convertTo8bitGrayscaleData(depthPixels.getWidth(), depthPixels.getHeight(), data)
      // load grayscale data into our ofTexture instance
      ->template convert<void>([&depthPixels, &depthTex](const void* data, size_t size){
        // ofLogNotice() << "buffer to tex onversion update: " << size;
        depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
        depthTex.loadData(depthPixels);
      });
  }

  void loadGrayscaleTexture(persee::Buffer& buf, ofTexture& tex) {
    // check if buffer has data
    persee::emptyAndInflateBuffer(buf, [&tex](const void* data, size_t size){
      loadGrayscaleTexture(tex, data, size);
    });
  }

  /**
   * Load data into ofTexture, assuming the data contains 3-channel color data
   */
  void loadColorTexture(ofTexture& tex, const void* data, size_t size) {
    ofPixels pixels;

    // allocate
    if(tex.isAllocated()) {
      pixels.allocate(tex.getWidth(), tex.getHeight(), OF_IMAGE_COLOR);
    } else {
      // TODO; infer texture size from receiver frame size?
      pixels.allocate(1280, 720, OF_IMAGE_COLOR);
      tex.allocate(pixels);
    }

    // check
    if((tex.getWidth() * tex.getHeight()) * 3 != size) {
      ofLogWarning() << "Color texture size did not match data-size";
      return;
    }

    // load
    pixels.setFromPixels((const unsigned char *)data, pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR);
    tex.loadData(pixels);
  }

  void loadColorTexture(persee::Buffer& buffer, ofTexture& tex) {
    // check if buffer has data
    persee::emptyAndInflateBuffer(buffer, [&tex](const void* data, size_t size){
      loadColorTexture(tex, data, size);
    });
  }
}
