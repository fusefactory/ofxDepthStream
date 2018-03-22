#pragma once

#include "Frame.h"

namespace depth {
  class Buffer {
    public: // sub-types

      typedef std::function<void(const void*, size_t)> NewDataCallback;

    public: // read methods

      virtual FrameRef getRef(){ return buffered; }

    public: // write methods

      virtual void write(FrameRef f) { this->write(f->data(), f->size()); }

      virtual void write(const void* data, size_t size) {
        buffered = Frame::refToExternalData(data,size);
        if(newDataCallback) newDataCallback(data, size);
      }

      void clear() { buffered=nullptr; }

    public: // config methods

      void setOutputTo(Buffer* b) {
        if(b)
          newDataCallback = [b](const void* data, size_t size){
            b->write(data, size);
          };
        else
          newDataCallback = nullptr;
      }

    protected: // attributes

      NewDataCallback newDataCallback=nullptr;
      FrameRef buffered=nullptr;
  };
}
