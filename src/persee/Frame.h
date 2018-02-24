#pragma once

#include <memory>

namespace persee {

  class Frame;
    typedef std::shared_ptr<Frame> FrameRef;


  class Frame {
    public: // static methods

      typedef std::function<void(const void*, size_t)> InputFunc;
      static FrameRef ref(size_t size) { return std::make_shared<Frame>(size); }
      static FrameRef refToExternalData(const void* data, size_t size) {
        return std::make_shared<Frame>(data, size);
      }

    public: // constructors

      // self-allocating
      Frame(size_t size) : _size(size) { ownedData = malloc(size); }
      // pre-initialized frame
      Frame(const void* data, size_t size) : _size(size), externalData(data) { }

      ~Frame() { if(ownedData) free(ownedData); }

    public: // getters/setters

      const void* data() { return ownedData ? ownedData : externalData; }
      size_t size() { return _size; }

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
