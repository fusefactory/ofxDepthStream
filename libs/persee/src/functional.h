#pragma once

#include "Frame.h"
#include "Buffer.h"
#include "Inflater.h"
#include "Compressor.h"

namespace persee {

  // compress methods

  FrameRef compress(const void* data, size_t size) {
    Compressor compressor;
    if(!compressor.compress(data, size)) return nullptr;
    return Frame::refToExternalData(compressor.getData(), compressor.getSize());
  }

  FrameRef compress(Frame& instance) { return compress(instance.data(), instance.size()); }
  FrameRef compress(FrameRef ref) { return compress(ref->data(), ref->size()); }


  FrameRef inflate(const void* data, size_t size) {
    Inflater inf;
    if(!inf.inflate(data, size)) return nullptr;
    return Frame::refToExternalData(inf.getData(), inf.getSize());
  }

  // inflate methods

  FrameRef inflate(Frame& instance) { return inflate(instance.data(), instance.size()); }
  FrameRef inflate(FrameRef ref) { return inflate(ref->data(), ref->size()); }

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

  void emptyAndInflateBuffer(Buffer& buf, Frame::InputFunc func) {
    emptyBuffer(buf, [func](const void* data, size_t size){
      auto inflatedFrameRef = inflate(data, size);

      if(!inflatedFrameRef) {
        std::cerr << "Could not inflate " << size << "-byte buffer data" << std::endl;
        return;
      }

      func(inflatedFrameRef->data(), inflatedFrameRef->size());
    });
  }

}
