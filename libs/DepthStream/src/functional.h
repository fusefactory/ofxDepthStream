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
 * \file This file defines the functional-programming interface to the DepthStream API
 */

#pragma once

#include "Frame.h"
#include "Buffer.h"
#include "Inflater.h"
#include "Compressor.h"

/**
 * \namespace depth contains all classes and functions of the DepthStream library.
 *
 * The DepthStream library provides tool for streaming (transmitteing and receiving)
 * compressed image-data of a network.
 */
namespace depth {

  struct Opts {
    InflaterRef inflaterRef=nullptr;
    Opts& useInflater(InflaterRef inflaterRef) { this->inflaterRef = inflaterRef; return *this; }
  };

  // compress methods
  FrameRef compress(const void* data, size_t size);
  FrameRef compress(Frame& instance);
  FrameRef compress(FrameRef ref);

  // inflate (decompress) methods
  FrameRef inflate(const void* data, size_t size, InflaterRef reusableInflater=nullptr);
  FrameRef inflate(Frame& instance);
  FrameRef inflate(FrameRef ref);

  // bit-size conversion
  FrameRef convert_32bit_to_8bit(size_t targetSize, const void* data);
  FrameRef convert_16bit_to_8bit(size_t targetSize, const void* data);

  // grayscale methods
  FrameRef convertTo8bitGrayscaleData(size_t texSize, const void* data);
  FrameRef convertTo8bitGrayscaleData(int texWidth, int texHeight, const void* data);
  FrameRef convertTo8bitGrayscaleData(int texWidth, int texHeight, FrameRef ref);

  /// Generates converter lambda for specified width/height
  std::function<FrameRef(const void*, size_t)> grayscale8bitConverter(int texWidth, int texHeight);

  // buffer methods
  void emptyBuffer(Buffer& buf, Frame::InputFunc func);
  void emptyAndInflateBuffer(Buffer& buf, Frame::InputFunc func, const Opts& opts = Opts());
}
