#pragma once

namespace persee {
  class Inflater;
  typedef std::shared_ptr<Inflater> InflaterRef;

  class Inflater {
    public:
      ~Inflater(){
        destroy();
      }

      void destroy();

      bool inflate(char* data, size_t size);
      size_t getSize() const { return inflateSize; }
      const char* getData() const { return decompressed; }

    protected:

      void growTo(size_t to);
      char *decompress(char *compressedBytes, unsigned int length);

      std::ostream& cout() { return std::cout << "[persee::Inflater] "; }
      std::ostream& cerr() { return std::cerr << "[persee::Inflater] "; }

    private:
      char* decompressed=NULL;
      size_t currentBufferSize=0;
      size_t inflateSize=0;
  };
}
