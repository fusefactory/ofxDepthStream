//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include <string.h>
#ifdef _WIN32
	#include <io.h>

	//#include "zlibdll/include/zlib.h"
	// #pragma comment(lib, "zlibdll/lib/zdll.lib")
	#include "zlib.h"
#else
	#include <unistd.h>
	#include "zlib.h"
#endif
#include <stdio.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>
#include "Inflater.h"

#define DEFAULT_BUF_SIZE (1280*720*4)

using namespace std;
using namespace depth;

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
	if (this->decompressed) {
  	memcpy(tmp, (char*)decompressed, currentBufferSize);
    destroy();
  }
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
      this->growTo(DEFAULT_BUF_SIZE);
    }

    z_stream strm;
    strm.next_in = (Bytef *)compressedBytes;
    strm.avail_in = length;
    strm.total_out = 0;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    bool done = false ;

    if (inflateInit(&strm) != Z_OK) {
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
        // return NULL;
        break;
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
