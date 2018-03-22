#pragma once

// OF
#include "ofMain.h"
// local
#include "DepthStream.h"

namespace ofxDepthStream {

  static const size_t FRAME_SIZE_640x480x16BIT = (640*480*2); // orbbec
  static const size_t FRAME_SIZE_640x480x32BIT = (640*480*4);
  static const size_t FRAME_SIZE_512x424x32BIT = (512*424*4); // kinect

  // Depth texture loader methods // // // // //

  struct DepthLoaderOpts {
    int minDistance=0;
    int maxDistance=5000;
    int vertCorrection=0; // 1?
    int shift1=0, shift2=0;
    float keystone=0.0f;
    float margins[4]={0.0f, 0.0f, 0.0f, 0.0f};

    DepthLoaderOpts& setMinDistance(int v) { minDistance = v; return *this; }
    DepthLoaderOpts& setMaxDistance(int v) { maxDistance = v; return *this; }
    DepthLoaderOpts& setVertCorrection(int v) { vertCorrection = v; return *this; }
    DepthLoaderOpts& setMargins(const float* v) { margins[0] = v[0]; margins[1] = v[1]; margins[2] = v[2]; margins[3] = v[3]; return *this; }
    DepthLoaderOpts& setMarginTop(float v) { margins[0] = v; return *this; }
    DepthLoaderOpts& setMarginRight(float v) { margins[1] = v; return *this; }
    DepthLoaderOpts& setMarginBottom(float v) { margins[2] = v; return *this; }
    DepthLoaderOpts& setMarginLeft(float v) { margins[3] = v; return *this; }
    DepthLoaderOpts& setKeystone(float v) { keystone = v; return *this; }
    DepthLoaderOpts& setShift1(int v) { shift1 = v; return *this; }
    DepthLoaderOpts& setShift2(int v) { shift2 = v; return *this; }
  };

  /**
   * loadEdgeData based on KinectRemote::newData method in the Dokk_OF repo (but converted to 16-bit)
   */
  void loadDepthTexture16bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts()) {
    // allocate
    if(!tex.isAllocated()) {
      // ofLogNotice() << "Allocating edge-data texture";
      if(size == FRAME_SIZE_640x480x16BIT) {
        tex.allocate(640, 480, GL_RGB);
      } else {
        ofLogWarning() << "Frame-size not supported by ofxDepthStream::loadDepthTexture16bit: " << size;
        return;
      }
    }

    // // check
    // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
    // if(expectedSize != size) {
    //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
    //   return;
    // }

    // 3-channel data buffer
    float edgeData[(int)tex.getWidth() * (int)tex.getHeight() * 3];

    for (int y = 0.0; y < tex.getHeight(); y++) {
      for (int x = 0.0; x < tex.getWidth(); x++) {
        // keystone
        int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * opts.keystone * (x - tex.getWidth() / 2.0);

        // convert multi-byte into single depth value
        int depthIndex = int((posX + y * tex.getWidth()) * 2);
        // unsigned int byte0 = ((unsigned char*)data)[depthIndex + 0];
        // unsigned int byte1 = ((unsigned char*)data)[depthIndex + 1];
        // unsigned int depth = ((byte0 & 0xFF) << (8 + opts.shift1)) | ((byte1 & 0xFF) << opts.shift2);
        uint16_t depth = *((uint16_t*)(&((char*)data)[depthIndex]));
        int edgeIndex = (x + y * tex.getWidth());
        bool valid = false;

        if (
          // top margin
          y >= opts.margins[0]
          // left margin
          && posX >= opts.margins[3]
          // right margin
          && posX <= tex.getWidth() - opts.margins[1]
          // bottom margin
          && y <= tex.getHeight() - opts.margins[2])
        {
          int correctMaxDistance = opts.maxDistance * (1.0 - opts.vertCorrection * (std::cos(M_PI / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
          if (depth >= opts.minDistance && depth <= correctMaxDistance) {
            float intensity = (depth - opts.minDistance) / (float)(correctMaxDistance - opts.minDistance);
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
  void loadDepthTexture32bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts()) {
    // allocate
    if(!tex.isAllocated()) {
      // ofLogNotice() << "Allocating edge-data texture";
      if(size == FRAME_SIZE_640x480x32BIT) {
        tex.allocate(640, 480, GL_RGB);
      } else if(size == FRAME_SIZE_512x424x32BIT){
        tex.allocate(512, 424, GL_RGB);
      } else {
        ofLogWarning() << "Frame-size not supported by ofxDepthStream::loadDepthTexture32bit: " << size;
        return;
      }
    }

    // // check
    // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
    // if(expectedSize != size) {
    //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
    //   return;
    // }

    // 3-channel data buffer
    float edgeData[(int)tex.getWidth() * (int)tex.getHeight() * 3];

    for (int y = 0.0; y < tex.getHeight(); y++) {
      for (int x = 0.0; x < tex.getWidth(); x++) {
        // keystone
        int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * opts.keystone * (x - tex.getWidth() / 2.0);

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
          y >= opts.margins[0]
          // left margin
          && posX >= opts.margins[3]
          // right margin
          && posX <= tex.getWidth() - opts.margins[1]
          // bottom margin
          && y <= tex.getHeight() - opts.margins[2])
        {
          int correctMaxDistance = opts.maxDistance * (1.0 - opts.vertCorrection * (std::cos(M_PI / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
          if (depth >= opts.minDistance && depth <= correctMaxDistance) {
            float intensity = (depth - opts.minDistance) / (float)(correctMaxDistance - opts.minDistance);
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
   * and calls the appropriate converter method
   */
  void loadDepthTexture(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts()) {
    if (size == FRAME_SIZE_640x480x16BIT) {
      loadDepthTexture16bit(tex, data, size, opts);
      return;
    }

    if (size == FRAME_SIZE_640x480x32BIT || size == FRAME_SIZE_512x424x32BIT) {
      loadDepthTexture32bit(tex, data, size, opts);
      return;
    }

    ofLogWarning() << "Frame size not supported by ofxDepthStream::loadDepthTexture (bytes): " << size;
  }

  void loadDepthTexture(depth::Buffer& buffer, ofTexture& tex, const DepthLoaderOpts& opts = DepthLoaderOpts()) {
    // check if buffer has data
    depth::emptyAndInflateBuffer(buffer, [&tex, &opts](const void* data, size_t size){
      loadDepthTexture(tex, data, size, opts);
    });
  }

  // Mesh Loader methods // // // // //

  struct MeshLoaderOpts {
    float depthFactor=-1.0f;
    MeshLoaderOpts& setDepthFactor(float v) { depthFactor = v; return *this; }
  };

  void loadMesh16bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts = MeshLoaderOpts()) {
    const uint16_t* pointData = (const uint16_t*)data;

    mesh.clear();

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        uint16_t val = pointData[y*width+x];
        ofVec3f p(x,y, val * opts.depthFactor);
        mesh.addVertex(p);

        // TODO; provide some coloring options
        float hue  = ofMap(val, 0, 6000, 0, 255);
        mesh.addColor(ofColor::fromHsb(hue, 255, 255));
      }
    }
  }

  /** UNTESTED */
  void loadMesh32bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts = MeshLoaderOpts()) {
    const uint32_t* pointData = (const uint32_t*)data;

    mesh.clear();

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        uint32_t val = pointData[y*width+x]; // TODO/TOTEST; do manual byte processing here?
        ofVec3f p(x,y, val * opts.depthFactor);
        mesh.addVertex(p);

        // TODO; provide some coloring options
        float hue  = ofMap(val, 0, 6000, 0, 255);
        mesh.addColor(ofColor::fromHsb(hue, 255, 255));
      }
    }
  }

  void loadMesh(ofMesh& mesh, const void* data, size_t size, const MeshLoaderOpts& opts = MeshLoaderOpts()) {
    if(size == FRAME_SIZE_640x480x16BIT) {
      loadMesh16bit(mesh, data, 640, 480, opts);
      return;
    }

    if(size == FRAME_SIZE_640x480x32BIT) {
      loadMesh32bit(mesh, data, 640, 480, opts);
      return;
    }

    if(size == FRAME_SIZE_512x424x32BIT){
      loadMesh32bit(mesh, data, 512, 424, opts);
      return;
    }

    ofLogWarning() << "Frame size not supported by ofxDepthStream::loadMesh (bytes): " << size;
  }

  void loadMesh(depth::Buffer& buffer, ofMesh& mesh, const MeshLoaderOpts& opts = MeshLoaderOpts()) {
    // check if buffer has data
    depth::emptyAndInflateBuffer(buffer, [&mesh, &opts](const void* data, size_t size){
      loadMesh(mesh, data, size, opts);
    });
  }

  // Grayscale texture loader methods deprecated? Use depth loader methods)  // // // // //

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
      // returns shared_ptr<depth::Frame> with 1-byte grayscale data
      depth::convert_16bit_to_8bit(pixelCount, data)
      // load grayscale data into our ofTexture instance
      ->template convert<void>([&depthPixels, &tex](const void* data, size_t size){
        // ofLogNotice() << "buffer to tex onversion update: " << size;
        depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
        tex.loadData(depthPixels);
      });

      return;
    }

    if (size == size32bit) {
      // returns shared_ptr<depth::Frame> with 1-byte grayscale data
      depth::convert_32bit_to_8bit(pixelCount, data)
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

  void loadGrayscaleTexture(depth::Buffer& buf, ofTexture& tex) {
    // check if buffer has data
    depth::emptyAndInflateBuffer(buf, [&tex](const void* data, size_t size){
      loadGrayscaleTexture(tex, data, size);
    });
  }

  // Color texture loader methods (untested) // // // // //

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

  void loadColorTexture(depth::Buffer& buffer, ofTexture& tex) {
    // check if buffer has data
    depth::emptyAndInflateBuffer(buffer, [&tex](const void* data, size_t size){
      loadColorTexture(tex, data, size);
    });
  }
}
