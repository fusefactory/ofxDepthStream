#pragma once

#include <memory>

namespace persee {

  /**
   * Frame is a simple read-only wrapper around a data block and a size attribute.
   * Frames can be initialized both with embedded data buffer (which will be
   * deallocated by the Frame's destructor) and with external data.
   */

  class Frame;
  typedef std::shared_ptr<Frame> FrameRef;

  class Frame {
    public: // static methods

      typedef std::function<void(const void*, size_t)> InputFunc;
      static FrameRef ref(size_t size) { return std::make_shared<Frame>(size); }
      static FrameRef refToExternalData(const void* data, size_t size) {
        return std::make_shared<Frame>(data, size);
      }
      static FrameRef refWithData(void* data, size_t size) {
        auto ref = std::make_shared<Frame>();
        ref->_size = size;
        ref->ownedData = data;
        return ref;
      }

    public: // constructors

      Frame(){}
      // self-allocating
      Frame(size_t size) : _size(size) { ownedData = malloc(size); }
      // pre-initialized frame
      Frame(const void* data, size_t size) : _size(size), externalData(data) { }

      ~Frame() { if(ownedData) free(ownedData); }

    public: // getters

      const void* data() { return ownedData ? ownedData : externalData; }
      size_t size() { return _size; }

      /**
       * The concert methods takes a function (ie. lambda) and, executes it
       * and returns whatever type the lambda returns. This allows linking
       * operations (frame->convert(...)->convert(...)-> etc. 
       */
      template<typename ResultType>
      ResultType convert(std::function<ResultType(const void*,size_t)> func) {
        return func(data(), size());
      }

    private: // attributes

      size_t _size;
      void* ownedData=NULL;
      const void* externalData=NULL;
  };
}
