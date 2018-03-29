#include <iostream>
#include "zlib.h"
#include "Compressor.h"


using namespace depth;

bool Compressor::compress(const void* data, size_t size) {
  // memcpy(buffer, data, size);
  if(data){
    this->size = size;
    this->buffer = (const unsigned char*)data;

    // std::string ss(data, size);
    // std::cout << "compressing: " << ss << std::endl;
    return doCompression();
  }

  std::cout << "Got NULL pointer to compress" << std::endl;
  return false;
}

bool Compressor::doCompression() {
  // zlib struct
  z_stream defstream;
  defstream.zalloc = Z_NULL;
  defstream.zfree = Z_NULL;
  defstream.opaque = Z_NULL;
  // setup "a" as the input and "b" as the compressed output
  defstream.avail_in = (uInt)this->size; // size of input, string + terminator
  defstream.next_in = (Bytef *)buffer; // input char array
  defstream.avail_out = (uInt)BUF_SIZE-1; // size of output
  defstream.next_out = (Bytef *)compressed; // output char array

  // // the actual compression work.
  deflateInit(&defstream, Z_BEST_SPEED);
  deflateResult = deflate(&defstream, Z_FINISH);
  deflateEnd(&defstream);

  if(deflateResult != Z_STREAM_END) {
    std::cout << "couldn't finish deflation" << std::endl;
    return false;
  }

  // sizeCompressed = strlen(compressed);
  sizeCompressed = defstream.total_out;
  // This is one way of getting the size of the output
  // std::cout << "Compressed from " << size << " to " << sizeCompressed << " bytes" << std::endl;
  // printf("Compressed string is: %s\n", compressed);
  return true;
}
