#pragma once

#include "Frame.h"

namespace depth {
  /**
   * \brief Manages a reference to a Frame instance
   *
   * The Buffer class only manages a reference to a Frame instance, and allows
   * the owner register a callback to be notified when the buffer is being written to.
   **/
  class Buffer {
    public: // sub-types

      /// A new data callback takes a void pointer to the memory block and a size_t with the number of bytes in the memory block
      typedef std::function<void(const void*, size_t)> NewDataCallback;

    public: // read methods

      /// @return a shared_ptr to the buffered Frame (can return a nullptr)
      virtual FrameRef getRef(){ return buffered; }

    public: // write methods

      /// Updates the buffer with a new frame (triggers the output callback)
      virtual void write(FrameRef f) { this->write(f->data(), f->size()); }

      /// Updates the buffer with a new frame (triggers the output callback)
      virtual void write(const void* data, size_t size) {
        buffered = Frame::refToExternalData(data,size);
        if(newDataCallback) newDataCallback(data, size);
      }

      /// Clear the buffer (doesn't trigger the output callback)
      void clear() { buffered=nullptr; }

    public: // config methods

      /// Registers an output callback that updates the given buffer with this buffer's data whenever this buffer is updated
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
