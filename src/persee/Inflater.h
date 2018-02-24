#pragma once

#include <memory>

namespace persee {
  class Inflater;
  typedef std::shared_ptr<Inflater> InflaterRef;

  class Inflater {
    public:
      ~Inflater(){
        destroy();
      }

      void destroy();

      bool inflate(const void* data, size_t size);
      size_t getSize() const { return inflateSize; }
      const void* getData() const { return (void*)decompressed; }
      void* releaseData(){ void* tmp = (void*)decompressed; decompressed=NULL; return tmp; }

    protected:

      void growTo(size_t to);
      const void* decompress(const void* compressedBytes, unsigned int length);

      std::ostream& cout() { return std::cout << "[persee::Inflater] "; }
      std::ostream& cerr() { return std::cerr << "[persee::Inflater] "; }

    private:
      char* decompressed=NULL;
      size_t currentBufferSize=0;
      size_t inflateSize=0;
  };
}
