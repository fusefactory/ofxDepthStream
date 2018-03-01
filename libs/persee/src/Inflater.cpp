#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>

#include "zlib.h"
#include "Inflater.h"

#define BUF_SIZE (2024*2024*4)

using namespace std;
using namespace persee;

void Inflater::destroy() {
  if(decompressed){
    free(decompressed);
    decompressed = NULL;
    currentBufferSize=0;
  }
}

bool Inflater::inflate(const void* data, size_t size) {
  return this->decompress(data, size) != NULL;
}

void Inflater::growTo(size_t to) {
  char* tmp = (char *) calloc( sizeof(char), to);
  memcpy(tmp, (char*)decompressed, currentBufferSize);
  destroy();
  decompressed = tmp;
  currentBufferSize = to;
  if(bVerbose) this->cout() << "grown to " << currentBufferSize << " bytes" << std::endl;
}

const void* Inflater::decompress(const void* compressedBytes, unsigned int length) {
    if (length == 0) {
        this->cout() << "length = 0, nothing to decompress" << std::endl;
        return NULL;
    }

    // destroy();
    if (!decompressed) {
      if(bVerbose) this->cout() << "allocating first-time inflation buffer" << std::endl;
      decompressed = (char *) calloc(sizeof(char), BUF_SIZE);
      currentBufferSize = BUF_SIZE;
    }

    z_stream strm;
    strm.next_in = (Bytef *)compressedBytes;
    strm.avail_in = length ;
    strm.total_out = 0;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;

    bool done = false ;

    if (inflateInit2(&strm, MAX_WBITS) != Z_OK) {
        this->cerr() << "inflator init failed" << std::endl;
        failCount++;
        return NULL;
    }

    while (!done) {
        // if our output buffer is too small
        if (strm.total_out >= currentBufferSize ) {
          this->growTo(currentBufferSize+length);
        }

        strm.next_out = (Bytef *) (decompressed + strm.total_out);
        strm.avail_out = currentBufferSize - strm.total_out;

        // inflate another chunk
        int err = ::inflate (& strm, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) {
          done = true;
          // this->cout() << "inflated packet to: " << strm.total_out << " bytes" << std::endl;
        }
        else if (err != Z_OK)  {
          if(bVerbose) this->cerr() << "Inflation failed; unknown error" << std::endl;
          failCount++;
          // perror("perror");
          return NULL;
          // break;
        }
    }

    if (inflateEnd (& strm) != Z_OK) {
        this->cerr() << "inflate end with non-OK result" << std::endl;
        failCount++;
        return NULL;
    }

    // this->cout() << "inflated to total: " << strm.total_out << " bytes" << std::endl;
    inflateSize = strm.total_out;
    return decompressed;
}
