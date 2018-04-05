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

// By default this we'll assume APPLE is a development environment
// without OpenNI and OpenCV.
#ifndef __APPLE__
  #define OPENNI_AVAILABLE
  #define OPENCV_AVAILABLE
#endif

// stdlib
#include <iostream>
#include <vector>
#include <chrono>
// local
#include "../../../libs/DepthStream/src/OniSampleUtilities.h"
#include "../../../libs/DepthStream/src/CamInterface.h"
#include "../../../libs/DepthStream/src/Compressor.h"
#include "../../../libs/DepthStream/src/Transmitter.h"

using namespace std;
using namespace std::chrono;
using namespace depth;

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

int main(int argc, char** argv) {
  unsigned int sleepTime = 5; // ms
  int depthPort = 4445;
  int fps = 60;
  bool bVerbose=false;
  std::shared_ptr<Converter16to32bit> converterRef = nullptr;

  // process command-line arguments
  for(int i=1; i<argc; i++) {
    if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
      bVerbose=true;
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

    if(strcmp(argv[i], "--sleep-time") == 0 || strcmp(argv[i], "-s") == 0) {
      sleepTime = atoi(argv[i+1]);
      i++;
      continue;
    }

    std::cerr << "Unknown argument: " << argv[i] << std::endl;
  }

  // attributes
  auto compressor = std::make_shared<Compressor>();
  std::vector<std::shared_ptr<Transmitter>> depthStreamTransmitters;
  std::vector<std::shared_ptr<Transmitter>> colorStreamTransmitters;
  std::shared_ptr<depth::VideoStream> depth=nullptr, color=nullptr;

  // setup camera feed
  depth::CamInterface camInt;
  depth = camInt.getDepthStream();

  // setup streamers
  if(depthPort > 0) {
    std::cout << "Starting depth transmitter on port " << depthPort << std::endl;
    depthStreamTransmitters.push_back(std::make_shared<Transmitter>(depthPort));
  }

  // config timing
  auto nextFrameTime = steady_clock::now();
  unsigned int frameMs = (int)(1.0f/(float)fps * 1000.0f); // milliseconds

  // main loop; send frames
  while (!wasKeyboardHit()) {
    auto t = steady_clock::now();

    // time for next frame?
    if (t >= nextFrameTime) {
      if(bVerbose) std::cout << "time for new frame" << std::endl;

      auto stream = camInt.getReadyStream();
      if(stream) {
        if(bVerbose) std::cout << "found stream with new content" << std::endl;
        // schedule next frame
        nextFrameTime = t + std::chrono::milliseconds(frameMs);

        std::string name;
        std::vector<std::shared_ptr<Transmitter>>* transmitters;
        if(stream == depth){
          transmitters = &depthStreamTransmitters;
          name = "depth";
        } else {
          transmitters = &colorStreamTransmitters;
          name = "color";
        }

        stream->update();

        // raw frame data
        const void* data = stream->getData();
        size_t size = stream->getSize();

        // convert?
        if(converterRef) {
          if(converterRef->convert(data, size)) {
            if(bVerbose) std::cout << "converting from 16-bit to 32-bit" << std::endl;
            data = converterRef->getData();
            size = converterRef->getSize();
            // std::cout << "converted to 32bit, size: " << size << std::endl;
          } else {
            data = NULL;
            std::cerr << "16 to 32 bit conversion failed" << std::endl;
          }
        }

        // conversion didn't fail
        if(!data) {
          std::cout << "FAILED to compress " << depth->getSize() << "-byte " << name << " frame" << std::endl;
        } else {
          // compress
          if(compressor->compress(data, size)) {
            // transmit through all transmitters (should be only one)
            for(auto t : (*transmitters)) {
              if(t->transmit((const char*)compressor->getData(), compressor->getSize())){
                // log if in verbose mode
                if(bVerbose) std::cout << "sent " << compressor->getSize() << "-byte " << name << " frame" << std::endl;
              }
            }
          }
        }
      }
    }

    Sleep(sleepTime);
  }

  std::cout << "cleaning up..." << std::endl;
  if(depth){
    depth->stop();
    depth->destroy();
  }
  if(color){
    color->stop();
    color->destroy();
  }
  camInt.close();

  return 0;
}
