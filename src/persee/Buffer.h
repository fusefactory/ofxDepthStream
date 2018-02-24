#pragma once

#include "Frame.h"

namespace persee {
  class Buffer {
  public:
    typedef std::function<void(const void*, size_t)> NewDataCallback;

  public:
    virtual void take(FrameRef f) { this->take(f->data(), f->size()); }

    virtual void take(const void* data, size_t size) {
      buffered = Frame::refToExternalData(data,size);
      if(newDataCallback) newDataCallback(data, size);
    }

    virtual FrameRef getRef(){ return buffered; }

    void outputTo(Buffer& b) {
      newDataCallback = [&b](const void* data, size_t size){
        b.take(data, size);
      };
    }

    void clear() { buffered=nullptr; }


  protected:
    NewDataCallback newDataCallback=nullptr;
  protected:
    FrameRef buffered=nullptr;
  };
}
