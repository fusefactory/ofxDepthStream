#pragma once

#include <memory>
#include <functional>

namespace depth {

  class Frame;
  typedef std::shared_ptr<Frame> FrameRef;

  /**
   * \brief A read-only wrapper around a data block of a specified size
   *
   * Frame is a read-only wrapper around a data block pointer and a size attribute.
   * Frames can be initialized both with _owned_ data (which will be
   * deallocated by the Frame's destructor) as well as with _external_ data (which
   * will simply be abandoned by the Frame's destructor).
   */
  class Frame {

    public: // static methods

      typedef std::function<void(const void*, size_t)> InputFunc;

      /// Allocates a Frame for _owned_ data
      static FrameRef ref(size_t size) { return std::make_shared<Frame>(size); }

      /// Initializes a frame with _external_ data
      static FrameRef refToExternalData(const void* data, size_t size) {
        return std::make_shared<Frame>(data, size);
      }

      /// Initializes a Frame with _owned_ data ("adopts" the provided data)
      static FrameRef refWithData(void* data, size_t size) {
        auto ref = std::make_shared<Frame>();
        ref->_size = size;
        ref->ownedData = data;
        return ref;
      }

    public: // constructors

      /// Create an uninitialized, empty Frame instance
      Frame(){}

      /// Allocates size bytes of _owned_ data
      Frame(size_t size) : _size(size) { ownedData = malloc(size); }

      /// Initializes with _external_ data
      Frame(const void* data, size_t size) : _size(size), externalData(data) { }

      /// Deallocates _owned_ data (if any)
      ~Frame() { if(ownedData) free(ownedData); }

    public: // getters

      /// @return A pointer to the frame's data block (either owned or external), can be NULL
      const void* data() { return ownedData ? ownedData : externalData; }

      /// @return The size of the frame's data block (either owned or external) in bytes (can be zero)
      size_t size() { return _size; }

      /**
       * \brief execute the given function with the frame's current content
       *
       * This template function fascilitates specifically towards functional programming and
       * lets the owner "convert" the data by choosing a return-value type.
       * When converting to another Frame reference, linked notation is possible;
       * frame->convert<FrameRef>(scaleDown)->convert<FrameRef>(blackAndWhite)->convert<FrameRef>(blur)
       */
      template<typename ResultType>
      ResultType convert(std::function<ResultType(const void*,size_t)> func) {
        return func(data(), size());
      }

    private: // attributes

      size_t _size=0;
      void* ownedData=NULL;
      const void* externalData=NULL;
  };
}
