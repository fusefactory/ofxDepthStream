// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015 Orbbec 3D
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Be excellent to each other.

// Modifications copyright (C) 2018 Fuse srl

#include <astra/astra.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "key_handler.h"

#include "../../../libs/DepthStream/src/Compressor.h"
#include "../../../libs/DepthStream/src/Transmitter.h"

class Converter16to32bit {
  public:
    bool convert(const void* data, size_t size) {
      if((size * 2) > BUF_SIZE){
        std::cerr << "Convert: " << size << " bytes is too big for our buffer" << std::endl;
        return false;
      }

      memset(buffer, 0, BUF_SIZE);

      size_t index=0;
      size_t ttl = size/2;
      while(index < ttl) {
        uint16_t val = ((uint16_t*)data)[index];

        // std::cout << "convert iteration: " << index << "," << ttl << "siz2: " << sizeof(uint32_t) << std::endl;
        ((unsigned char*)buffer)[index * 4 + 0] = 0;
        ((unsigned char*)buffer)[index * 4 + 1] = 0;
        ((unsigned char*)buffer)[index * 4 + 2] = (unsigned char)(val >> 8 & 0xFF);
        ((unsigned char*)buffer)[index * 4 + 3] = (unsigned char)(val & 0xFF);

        index += 1;

        // ((uint32_t*)this->buffer)[index*2] = ((uint16_t*)data)[index];
        // ((uint16_t*)this->buffer)[index*2+1] = 0;
        // index += 1;
      }

      lastSize = size * 2; //destCursor;
      // std::cout << "Converting done, original size: " << size << ", ttl: " << ttl << ", last size: " << lastSize << std::endl;
      return true;
    }

    const void* getData(){ return (void*)buffer; }
    size_t getSize(){ return lastSize; }

  private:
    static const size_t BUF_SIZE = (1280*720*4);
    unsigned char buffer[BUF_SIZE];
    size_t lastSize=0;
};

class StreamingFrameListener : public astra::FrameListener
{
  private:
    using buffer_ptr = std::unique_ptr<int16_t []>;
    buffer_ptr buffer_;
    unsigned int lastWidth_;
    unsigned int lastHeight_;

  public:
    StreamingFrameListener(unsigned int port=4445, float fps=60.0f, std::shared_ptr<Converter16to32bit> converter=nullptr, bool verbose=false) : depthPort(port), converterRef(converter), bVerbose(verbose) {
        frameMs = 1.0f / fps * 1000.0f;
        nextFrameTime = std::chrono::system_clock::now();
        this->compressor = std::make_shared<depth::Compressor>();
        this->transmitter = std::make_shared<depth::Transmitter>(this->depthPort);
    }

    virtual void on_frame_ready(astra::StreamReader& reader, astra::Frame& frame) override {
        auto t = std::chrono::system_clock::now();

        // not yet time for next frame?
        if (t < nextFrameTime)
          return;

        nextFrameTime = t + std::chrono::milliseconds(frameMs);

        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

        if (depthFrame.is_valid())
        {
            this->transmitFrame(depthFrame.data(), depthFrame.byte_length());
        }
    }

    void transmitFrame(const void* data, size_t size) {
      // convert?
      if(converterRef) {
        if(converterRef->convert(data, size)) {
          if(bVerbose) std::cout << "converting from 16-bit to 32-bit" << std::endl;
          data = converterRef->getData();
          size = converterRef->getSize();
          // std::cout << "converted to 32bit, size: " << size << std::endl;
        } else {
          std::cerr << "16 to 32 bit conversion failed" << std::endl;
          return;
        }
      }

      if(!compressor->compress(data, size)) {
        std::cerr << "compression failed" << std::endl;
        return;
      }

      if (transmitter->transmit((const char*)compressor->getData(), compressor->getSize())) {
        if(bVerbose) std::cout << "transmitted " << compressor->getSize() << "-byte depth frame" << std::endl;
      } else {
        if(bVerbose) std::cout << "transmit of " << compressor->getSize() << "-byte depth frame FAILED (probably no connection)" << std::endl;
      }
    }

  private:
    // unsigned int sleepTime = 5; // ms
    int depthPort = 4445;
    // int colorPort = 4446;
    bool bVerbose=false;
    std::shared_ptr<Converter16to32bit> converterRef = nullptr;

    depth::CompressorRef compressor=nullptr;
    depth::TransmitterRef transmitter=nullptr;

    using clock_type = std::chrono::system_clock;
    unsigned int frameMs = (int)(1.0f/(float)60.0 * 1000.0f); // milliseconds
    std::chrono::time_point<clock_type> nextFrameTime;
};

int main(int argc, char** argv)
{
    bool verbose=false;
    unsigned int depthPort=4445;
    unsigned int fps=60.0f;
    std::shared_ptr<Converter16to32bit> converterRef=nullptr;

    // process command-line arguments
    for(int i=1; i<argc; i++) {
      if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
        verbose=true;
        continue;
      }

      if(strcmp(argv[i], "--convert-32bit") == 0 || strcmp(argv[i], "-c") == 0) {
        converterRef = std::make_shared<Converter16to32bit>();
        continue;
      }

      if(argc <= (i+1)) {
        std::cerr << "Didn't get value for " << argv[i] << std::endl;
        break;
      }

      if(strcmp(argv[i], "--depth-port") == 0 || strcmp(argv[i], "-d") == 0) {
        depthPort = atoi(argv[i+1]);
        i++;
        continue;
      }

      if(strcmp(argv[i], "--fps") == 0 || strcmp(argv[i], "-f") == 0) {
        fps = atoi(argv[i+1]);
        i++;
        continue;
      }

      std::cerr << "Unknown argument: " << argv[i] << std::endl;
    }

    astra::initialize();

    set_key_handler();

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();

    StreamingFrameListener listener(depthPort, fps, converterRef, verbose);

    reader.stream<astra::DepthStream>().start();

    std::cout << "depthStream -- hFov: "
              << reader.stream<astra::DepthStream>().hFov()
              << " vFov: "
              << reader.stream<astra::DepthStream>().vFov()
              << std::endl;

    reader.add_listener(listener);

    do
    {
		#ifdef _WIN32
			astra_update();
		#else
			astra_temp_update();
		#endif
    } while (shouldContinue);

    reader.remove_listener(listener);

    astra::terminate();
}
