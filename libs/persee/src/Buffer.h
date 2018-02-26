#pragma once

#include "Frame.h"

namespace persee {
  class Buffer {
  public: // sub-types
    typedef std::function<void(const void*, size_t)> NewDataCallback;

  public: // read methods
    virtual FrameRef getRef(){ return buffered; }

  public: // write methods
    virtual void take(FrameRef f) { this->take(f->data(), f->size()); }

    virtual void take(const void* data, size_t size) {
      buffered = Frame::refToExternalData(data,size);
      if(newDataCallback) newDataCallback(data, size);
    }

    void clear() { buffered=nullptr; }

  public: // config methods
    void setOutputTo(Buffer* b) {
      if(b)
        newDataCallback = [b](const void* data, size_t size){
          b->take(data, size);
        };
      else
        newDataCallback = nullptr;
    }

  protected: // attributes
    NewDataCallback newDataCallback=nullptr;
    FrameRef buffered=nullptr;
  };
}
