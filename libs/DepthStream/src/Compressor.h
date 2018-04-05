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

#pragma once

#include <memory>

namespace depth {
  class Compressor;
  typedef std::shared_ptr<Compressor> CompressorRef;

  /**
   * \brief Compressed a package (frame) of data using zlib compression
   */
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
