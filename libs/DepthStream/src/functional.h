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
 * \file This file provides the functional-programming interface to the DepthStream API
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

  FrameRef compress(const void* data, size_t size) {
    Compressor compressor;
    if(!compressor.compress(data, size)) return nullptr;
    return Frame::refToExternalData(compressor.getData(), compressor.getSize());
  }

  FrameRef compress(Frame& instance) { return compress(instance.data(), instance.size()); }
  FrameRef compress(FrameRef ref) { return compress(ref->data(), ref->size()); }


  FrameRef inflate(const void* data, size_t size, InflaterRef reusableInflater=nullptr) {
    Inflater inf;
    if(!inf.inflate(data, size)) return nullptr;
    return Frame::refToExternalData(inf.getData(), inf.getSize());
  }

  // inflate methods

  FrameRef inflate(Frame& instance) { return inflate(instance.data(), instance.size()); }
  FrameRef inflate(FrameRef ref) { return inflate(ref->data(), ref->size()); }

  // bit-size conversion
  FrameRef convert_32bit_to_8bit(size_t targetSize, const void* data) {
    void* converted = malloc(targetSize);
    double multiplier = 1.0 * (2^8) / (2^32);

    for(int i=0; i<targetSize; i++) {
      // unsigned char a = ((const unsigned char*)data)[i*2+1];
      // unsigned char b = ((const unsigned char*)data)[i*4];
      int byte0 = ((char*)data)[i*4 + 3];
      int byte1 = ((char*)data)[i*4 + 2];
      int byte2 = ((char*)data)[i*4 + 1];
      int byte3 = ((char*)data)[i*4 + 0];
      unsigned int val = byte0 << 24 | (byte1 & 0xFF) << 16 | (byte2 & 0xFF) << 8 | (byte3 & 0xFF);
      ((unsigned char*)converted)[i] = (unsigned char)((double)val / multiplier);
    }

    // when the returned  shared_ptr<Frame> deallocates, it will free the memory we gave it
    return Frame::refWithData(converted, targetSize);
  }

  FrameRef convert_16bit_to_8bit(size_t targetSize, const void* data) {
    void* converted = malloc(targetSize);
    double multiplier = 1.0 * (2^8) / (2^16);

    for(int i=0; i<targetSize; i++) {
      // unsigned char a = ((const unsigned char*)data)[i*2+1];
      // unsigned char b = ((const unsigned char*)data)[i*2]+1;
      // ((unsigned char*)converted)[i] = b; //(unsigned char)(((a << 8) | b) >> 8);//(unsigned char)val;

      int byte0 = ((char*)data)[i*2 + 0];
      int byte1 = ((char*)data)[i*2 + 1];
      unsigned int val = byte0 << 8 | (byte1 & 0xFF);
      ((unsigned char*)converted)[i] = (unsigned char)((double)val / multiplier);
    }

    // when the returned  shared_ptr<Frame> deallocates, it will free the memory we gave it
    return Frame::refWithData(converted, targetSize);
  }

  // grayscale methods

  FrameRef convertTo8bitGrayscaleData(size_t texSize, const void* data) {
    void* converted = malloc(texSize);

    for(int i=0; i<texSize; i++) {
      // unsigned char a = ((const unsigned char*)data)[i*2+1];
      unsigned char b = ((const unsigned char*)data)[i*2];
      ((unsigned char*)converted)[i] = b; //(unsigned char)(((a << 8) | b) >> 8);//(unsigned char)val;
    }

    // ofLogNotice() << "last value:" << (int)converted[texSize-1];

    // when ther returned  shared_ptr<Frame> deallocates, it will free the memory we gave it
    return Frame::refWithData(converted, texSize);
  }

  FrameRef convertTo8bitGrayscaleData(int texWidth, int texHeight, const void* data) {
    return convertTo8bitGrayscaleData(texWidth * texHeight * 1, data);
  }

  FrameRef convertTo8bitGrayscaleData(int texWidth, int texHeight, FrameRef ref) {
    return convertTo8bitGrayscaleData(texWidth, texHeight, ref->data());
  }

  /// Generates converter lambda for specified width/height
  std::function<FrameRef(const void*, size_t)> grayscale8bitConverter(int texWidth, int texHeight) {
    return [texWidth, texHeight](const void* data, size_t size){
      return convertTo8bitGrayscaleData(texWidth, texHeight, data);
    };
  }

  // buffer methods

  void emptyBuffer(Buffer& buf, Frame::InputFunc func) {
    if(auto r = buf.getRef()) {
      r->convert(func);
      buf.clear();
    }
  }

  void emptyAndInflateBuffer(Buffer& buf, Frame::InputFunc func, const Opts& opts = Opts()) {
    emptyBuffer(buf, [&opts, func](const void* data, size_t size){
      if (opts.inflaterRef) {
        ofLogNotice() << "using provided inflater";
      }
      // inf.inflate(data, size)
      auto inflatedFrameRef =
        // use provided inflater
        (opts.inflaterRef)
          // try to inflate using provided inflater
          ? (opts.inflaterRef->inflate(data, size)
            // put inflated data into a frame instance
            ? Frame::refToExternalData(opts.inflaterRef->getData(), opts.inflaterRef->getSize())
            : nullptr)
          // use our functional inflate interface
          : inflate(data, size);

      if(!inflatedFrameRef) {
        // std::cerr << "Could not inflate " << size << "-byte buffer data" << std::endl;
        return;
      }

      func(inflatedFrameRef->data(), inflatedFrameRef->size());
    });
  }

}
