#pragma once

namespace depth {
  class Compressor {
    public:
      bool compress(const void* data, size_t size);
      const void* getData() { return compressed; }
      int getSize(){ return sizeCompressed; }

    private:
      bool doCompression();

    private:
      static const int BUF_SIZE = 1280*720*4; // TODO; use dynamically allocated size
      // input
      const unsigned char* buffer; //[RECEIVE_BUF_SIZE];
      unsigned int size=0;
      // output
      unsigned char compressed[BUF_SIZE];
      unsigned int sizeCompressed=0;
      int deflateResult;
  };
}
