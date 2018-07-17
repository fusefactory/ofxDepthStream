//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

/**
 * \file This file provides a functional-programming interface to this addon's OpenFrameworks-specific API
 */
#pragma once

#include "ofMain.h"
#include "DepthStream.h"

/**
 * \namespace OpenFrameworks-specific for working with (mostly converting) DepthStream data
 */
namespace ofxDepthStream {

  static const size_t FRAME_SIZE_640x480x16BIT = (640*480*2); // orbbec
  static const size_t FRAME_SIZE_640x480x32BIT = (640*480*4);
  static const size_t FRAME_SIZE_512x424x32BIT = (512*424*4); // kinect
  static const size_t FRAME_SIZE_512x424x16BIT = (512*424*2); // kinect
  static const size_t FRAME_SIZE_640x240x08BIT = (640*240*1); // leap motion
  static const size_t FRAME_SIZE_1280x720x16BIT = (1280*720*2); // Intel RealSense D435

  // Depth texture loader methods // // // // //

  /// Options container for the loadDepth* methods
  struct DepthLoaderOpts : depth::Opts {
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

    DepthLoaderOpts& useInflater(depth::InflaterRef inflaterRef) { depth::Opts::useInflater(inflaterRef); return *this; }
  };

  /**
   * populates the texture instance with 16-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
   */
  void loadDepthTexture8bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts());

  /**
   * populates the texture instance with 16-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
   */
  void loadDepthTexture16bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts());

  /**
   * populates the texture instance with 32-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
   */
  void loadDepthTexture32bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts());

  /**
   * populates the texture instance with grayscale depth-image data from the given frame-data. It allocates the texture if necessary and guesses the bit-depth of the data based on the size of the package.
   */
  void loadDepthTexture(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts = DepthLoaderOpts());

  /**
   * Populates texture with depth data from the buffer's current content and empties the buffer.
   *
   * The texture is allocated if necessary, the resolution and bit-depth is guessed based on the size of the buffer's content.
   */
  void loadDepthTexture(depth::Buffer& buffer, ofTexture& tex, const DepthLoaderOpts& opts = DepthLoaderOpts());

  // Mesh Loader methods // // // // //

  struct MeshLoaderOpts {
    float depthFactor=-1.0f;
    MeshLoaderOpts& setDepthFactor(float v) { depthFactor = v; return *this; }
  };

  /**
   * populates the mesh instance with 16-bit depth-image data. It allocates the texture if necessary.
   */
  void loadMesh16bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts = MeshLoaderOpts());
  /**
   * UNTESTED populates the mesh instance with 32-bit depth-image data. It allocates the texture if necessary.
   */
  void loadMesh32bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts = MeshLoaderOpts());

  /**
   * populates the mesh instance with depth-image data.
   *
   * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size
   */
  void loadMesh(ofMesh& mesh, const void* data, size_t size, const MeshLoaderOpts& opts = MeshLoaderOpts());
  /**
   * populates the mesh instance with depth-image data taken from the buffer's current content (if any).
   *
   * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size.
   * It empties the buffer after using its content.
   */
  void loadMesh(depth::Buffer& buffer, ofMesh& mesh, const MeshLoaderOpts& opts = MeshLoaderOpts());

  // Color texture loader methods (untested) // // // // //

  /**
   * UNTESTED populates the texture with color-image data from the given frame-data. It allocates the texture if necessary.
   */
  void loadColorTexture(ofTexture& tex, const void* data, size_t size);

  /**
   * UNTESTED populates the texture with color-image data taken from the buffer's current content (if any).
   *
   * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size.
   * It empties the buffer after using its content.
   */
  void loadColorTexture(depth::Buffer& buffer, ofTexture& tex);
}
