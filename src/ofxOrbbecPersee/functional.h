#pragma once

// OF
#include "ofMain.h"
// local/persee
#include "persee.h"

namespace ofxOrbbecPersee {

  static const size_t FRAME_SIZE_640x480x16BIT = (640*480*2); // orbbec
  static const size_t FRAME_SIZE_640x480x32BIT = (640*480*4);
  static const size_t FRAME_SIZE_512x424x32BIT = (512*424*4); // kinect

  /**
   * Supported intput frame-sizes:
   * 640 x 480 x 2=614400 bytes (16-bit)
   * 640 x 480 x 4=1228800 bytes (32-bit)
   */
  void loadGrayscaleTexture(ofTexture& tex, const void* data, size_t size) {
    ofPixels depthPixels;

    // allocate
    if(tex.isAllocated()) {
      depthPixels.allocate(tex.getWidth(), tex.getHeight(), OF_IMAGE_GRAYSCALE);
    } else {
      // TODO; infer texture size from receiver frame size?
      ofLogWarning() << "TODO: infer depth texture resolution from data-size";
      depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
      tex.allocate(depthPixels);
    }

    // check
    size_t pixelCount = tex.getWidth() * tex.getHeight();
    size_t size16bit = pixelCount * 2;
    size_t size32bit = pixelCount * 4;

    if (size == size16bit) {
      // returns shared_ptr<persee::Frame> with 1-byte grayscale data
      persee::convert_16bit_to_8bit(pixelCount, data)
      // load grayscale data into our ofTexture instance
      ->template convert<void>([&depthPixels, &tex](const void* data, size_t size){
        // ofLogNotice() << "buffer to tex onversion update: " << size;
        depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
        tex.loadData(depthPixels);
      });

      return;
    }

    if (size == size32bit) {
      // returns shared_ptr<persee::Frame> with 1-byte grayscale data
      persee::convert_32bit_to_8bit(pixelCount, data)
      // load grayscale data into our ofTexture instance
      ->template convert<void>([&depthPixels, &tex](const void* data, size_t size){
        // ofLogNotice() << "buffer to tex onversion update: " << size;
        depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
        tex.loadData(depthPixels);
      });

      return;
    }

    ofLogWarning() << "Depth texture size did not match data-size (got: " << size << ", expected: " << size16bit << " or " << size32bit << ")";
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
      ofLogWarning() << "TODO: infer color texture resolution from data-size";
      pixels.allocate(1280, 720, OF_IMAGE_COLOR);
      tex.allocate(pixels);
    }

    // check
    if((tex.getWidth() * tex.getHeight()) * 3 != size) {
      ofLogWarning() << "Color texture size did not match data-size (got: " << size << ", expected: 1280x720x3=2764800)";
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

  /**
   * loadEdgeData based on KinectRemote::newData method in the Dokk_OF repo (but converted to 16-bit)
   */
  void loadDepthTexture16bit(ofTexture& tex, const void* data, size_t size) {
    // allocate
    if(!tex.isAllocated()) {
      // ofLogNotice() << "Allocating edge-data texture";
      if(size == FRAME_SIZE_640x480x16BIT) {
        tex.allocate(640, 480, GL_RGB);
      } else {
        ofLogWarning() << "Frame-size not supported by ofxOrbbecPersee::loadDepthTexture16bit: " << size;
        return;
      }
    }

    // // check
    // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
    // if(expectedSize != size) {
    //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
    //   return;
    // }

    // TODO; make all these params configurable?
    float keystone=0.0f;
    float margins[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // CSS-style; top, right, bottom, left;
    int minDistance=0;//792;
    int maxDistance=9999; //4979;
    int vertCorrection = 1;

    // 3-channel data buffer
    float edgeData[(int)tex.getWidth() * (int)tex.getHeight() * 3];

    for (int y = 0.0; y < tex.getHeight(); y++) {
      for (int x = 0.0; x < tex.getWidth(); x++) {
        // keystone
        int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * keystone * (x - tex.getWidth() / 2.0);

        // convert multi-byte into single depth value
        int depthIndex = int((posX + y * tex.getWidth()) * 2);
        int byte0 = ((unsigned char*)data)[depthIndex + 0];
        int byte1 = ((unsigned char*)data)[depthIndex + 1];
        int depth = byte0 << 8 | (byte1 & 0xFF);

        int edgeIndex = (x + y * tex.getWidth());
        bool valid = false;

        if (
          // top margin
          y >= margins[0]
          // left margin
          && posX >= margins[3]
          // right margin
          && posX <= tex.getWidth() - margins[1]
          // bottom margin
          && y <= tex.getHeight() - margins[2])
        {
          int correctMaxDistance = maxDistance * (1.0 - vertCorrection * (std::cos(M_PI / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
          if (depth >= minDistance && depth <= correctMaxDistance) {
            float intensity = (depth - minDistance) / (float)(correctMaxDistance - minDistance);
            edgeData[edgeIndex * 3 + 0] = 1 - intensity;
            edgeData[edgeIndex * 3 + 1] = 1 - intensity;
            edgeData[edgeIndex * 3 + 2] = 1 - intensity;
            valid = true;
          }
        }
        if (!valid) {
          edgeData[edgeIndex * 3 + 0] = 0.0;
          edgeData[edgeIndex * 3 + 1] = 0.0;
          edgeData[edgeIndex * 3 + 2] = 0.0;
        }
      }
    }

    tex.loadData((float*)edgeData, tex.getWidth(), tex.getHeight(), GL_RGB);
    // ofLogNotice() << "loaded edge data texture";//: maxFoundDist: " << maxFoundDist;
  }

  /**
   * loadEdgeData based on KinectRemote::newData method in the Dokk_OF repo
   */
  void loadDepthTexture32bit(ofTexture& tex, const void* data, size_t size) {
    // allocate
    if(!tex.isAllocated()) {
      // ofLogNotice() << "Allocating edge-data texture";
      if(size == FRAME_SIZE_640x480x32BIT) {
        tex.allocate(640, 480, GL_RGB);
      } else if(size == FRAME_SIZE_512x424x32BIT){
        tex.allocate(512, 424, GL_RGB);
      } else {
        ofLogWarning() << "Frame-size not supported by ofxOrbbecPersee::loadDepthTexture32bit: " << size;
        return;
      }
    }

    // // check
    // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
    // if(expectedSize != size) {
    //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
    //   return;
    // }

    // TODO; make all these params configurable?
    float keystone=0.0f;
    float margins[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // CSS-style; top, right, bottom, left;
    int minDistance=0;//792;
    int maxDistance=9999; //4979;
    int vertCorrection = 1;

    // 3-channel data buffer
    float edgeData[(int)tex.getWidth() * (int)tex.getHeight() * 3];

    for (int y = 0.0; y < tex.getHeight(); y++) {
      for (int x = 0.0; x < tex.getWidth(); x++) {
        // keystone
        int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * keystone * (x - tex.getWidth() / 2.0);

        // convert multi-byte into single depth value
        int depthIndex = int((posX + y * tex.getWidth()) * 4);
        int byte0 = ((unsigned char*)data)[depthIndex + 0];
        int byte1 = ((unsigned char*)data)[depthIndex + 1];
        int byte2 = ((unsigned char*)data)[depthIndex + 2];
        int byte3 = ((unsigned char*)data)[depthIndex + 3];
        int depth = byte0 << 24 | (byte1 & 0xFF) << 16 | (byte2 & 0xFF) << 8 | (byte3 & 0xFF);

        int edgeIndex = (x + y * tex.getWidth());
        bool valid = false;

        if (
          // top margin
          y >= margins[0]
          // left margin
          && posX >= margins[3]
          // right margin
          && posX <= tex.getWidth() - margins[1]
          // bottom margin
          && y <= tex.getHeight() - margins[2])
        {
          int correctMaxDistance = maxDistance * (1.0 - vertCorrection * (std::cos(M_PI / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
          if (depth >= minDistance && depth <= correctMaxDistance) {
            float intensity = (depth - minDistance) / (float)(correctMaxDistance - minDistance);
            edgeData[edgeIndex * 3 + 0] = 1 - intensity;
            edgeData[edgeIndex * 3 + 1] = 1 - intensity;
            edgeData[edgeIndex * 3 + 2] = 1 - intensity;
            valid = true;
          }
        }
        if (!valid) {
          edgeData[edgeIndex * 3 + 0] = 0.0;
          edgeData[edgeIndex * 3 + 1] = 0.0;
          edgeData[edgeIndex * 3 + 2] = 0.0;
        }
      }
    }

    tex.loadData((float*)edgeData, tex.getWidth(), tex.getHeight(), GL_RGB);
    // ofLogNotice() << "loaded edge data texture";//: maxFoundDist: " << maxFoundDist;
  }

  /**
   * Tries to guess the texture resolution/channel-depth based on frame size
   * and calls the appropriate converted method
   */
  void loadDepthTexture(ofTexture& tex, const void* data, size_t size) {
    if (size == FRAME_SIZE_640x480x16BIT) {
      loadDepthTexture16bit(tex, data, size);
      return;
    }

    if (size == FRAME_SIZE_640x480x32BIT) {
      loadDepthTexture32bit(tex, data, size);
      return;
    }

    if (size == FRAME_SIZE_512x424x32BIT) {
      loadDepthTexture32bit(tex, data, size);
      return;
    }

    ofLogWarning() << "Frame size not supported by ofxOrbbecPersee::loadDepthTexture (bytes): " << size;
  }

  void loadDepthTexture(persee::Buffer& buffer, ofTexture& tex) {
    // check if buffer has data
    persee::emptyAndInflateBuffer(buffer, [&tex](const void* data, size_t size){
      loadDepthTexture(tex, data, size);
    });
  }
}
