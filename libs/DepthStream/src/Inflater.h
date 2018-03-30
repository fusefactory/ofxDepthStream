#pragma once

#include <memory>

namespace depth {
  class Inflater;
  typedef std::shared_ptr<Inflater> InflaterRef;

  /**
   * \brief Inflates ("decompresses") a package ("frame") of data compressed using zlib doCompression
   */
  class Inflater {
    public:

      /// Deallocates the memory used to store inflated content (will not deallocate the memory that was release using the releaseData method)
      ~Inflater(){
        destroy();
      }

      /// Deallocates the memory used to store inflated content (will not deallocate the memory that was release using the releaseData method)
      void destroy();

      /// Performs decompression on the provided data package
      bool inflate(const void* data, size_t size);

      /// Returns the inflated size of the last inflate operation
      size_t getSize() const { return inflateSize; }

      /// Returns a pointer to the inflated package data (will be NULL when no inflation is performed or after releaseData is called)
      const void* getData() const { return (void*)decompressed; }

      /// "Releases" (abandons) and returns a pointer to the allocated data for decompression.
      /// When using this method, the caller is responsible for deallocating the returned memory block
      void* releaseData(){ void* tmp = (void*)decompressed; decompressed=NULL; return tmp; }

      /// Returns the number of times this instance failed to succesfully inflate a package
      size_t getFailCount() const { return failCount; }

    protected:

      void growTo(size_t to);
      const void* decompress(const void* compressedBytes, unsigned int length);

      std::ostream& cout() { return std::cout << "[depth::Inflater] "; }
      std::ostream& cerr() { return std::cerr << "[depth::Inflater] "; }

    private:
      char* decompressed=NULL;
      size_t currentBufferSize=0;
      size_t inflateSize=0;
      size_t failCount=0;
      bool bVerbose=false;
  };
}
