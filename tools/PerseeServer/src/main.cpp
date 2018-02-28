/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/

// stdlib
#include <iostream>
#include <vector>
#include <chrono>
// opencv2
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
// local
#include "config.h"
#include "../../../libs/persee/src/OniSampleUtilities.h"
#include "../../../libs/persee/src/CamInterface.h"
#include "../../../libs/persee/src/Compressor.h"
#include "../../../libs/persee/src/Transmitter.h"

using namespace std;
using namespace std::chrono;
using namespace cv;
using namespace persee;

struct ClrSrc {
  std::shared_ptr<VideoCapture> capRef;
  Mat frame;

  // ~ClrSrc(){
  //   cvReleaseImage(&frame);
  // }

};

std::shared_ptr<ClrSrc> createColorSource() {
  #ifndef OPENCV_AVAILABLE
    return nullptr;
  #else
    auto capRef = std::make_shared<VideoCapture>(CAP_OPENNI2); // open default camera

    if(!capRef->isOpened()) {
      std::cerr << "Could not open cv::VideoCapture device for color stream" << std::endl;
      // return nullptr;
    }

    auto ref = std::make_shared<ClrSrc>();
    ref->capRef = capRef;
    return ref;
  #endif
}

class Converter {
public:
  Converter(size_t srcBytes, size_t targetBytes) : fromBytes(srcBytes), toBytes(targetBytes){}

  bool convert(const void* data, size_t size) {
    if((float)size / fromBytes * toBytes > BUF_SIZE){
      std::cerr << "Convert: " << size << " bytes is too big for our buffer" << std::endl;
      return false;
    }

    if(toBytes < fromBytes) {
      std::cerr << "downscaling not supported (yet)" << std::endl;
      return false;
    }

    size_t diff = toBytes-fromBytes;
    size_t srcCursor=0, destCursor=0;
    while(srcCursor < size) {
      // prefix with zeroes
      memset((void*)&buffer[destCursor], 0, diff);
      // write source bytes
      memcpy((void*)&buffer[destCursor+diff], &((char*)data)[srcCursor], fromBytes);

      srcCursor += fromBytes;
      destCursor += toBytes;
    }

    lastSize = destCursor;
    return true;
  }

  const void* getData(){ return (void*)buffer; }
  size_t getSize(){ return lastSize; }

private:
  size_t fromBytes, toBytes;
  static const size_t BUF_SIZE = (1280*720*4);
  unsigned char buffer[BUF_SIZE];

  size_t lastSize=0;
};

int main(int argc, char** argv) {
  // configurables
  // bool bResendFrames = false;

  unsigned int sleepTime = 5; // ms
  int depthPort = argc > 1 ? atoi(argv[1]) : 4445;
  int colorPort = argc > 2 ? atoi(argv[2]) : 4446;
  int fps = argc > 3 ? atoi(argv[3]) : 12;
  float frameDiffTime = 1.0f/(float)fps * 1000.0f; // fps
  bool bVerbose=false;

  // attributes
  steady_clock::time_point lastFrameTime = steady_clock::now();
  auto compressor = std::make_shared<Compressor>();
  std::vector<std::shared_ptr<Transmitter>> depthStreamTransmitters;
  std::vector<std::shared_ptr<Transmitter>> colorStreamTransmitters;

  std::shared_ptr<persee::VideoStream> depth=nullptr, color=nullptr;

  persee::CamInterface camInt;

  depth = camInt.getDepthStream();
  std::shared_ptr<Converter> converterRef = std::make_shared<Converter>(2, 4); // 16-bits to 32-bits

  // color = camInt.getColorStream();
  auto clrSrc = createColorSource();


  if(depthPort > 0) {
    std::cout << "Starting depth transmitter on port " << depthPort << std::endl;
    depthStreamTransmitters.push_back(std::make_shared<Transmitter>(depthPort));
  }

  if(colorPort > 0){
    std::cout << "Starting color transmitter on port " << colorPort << std::endl;
    colorStreamTransmitters.push_back(std::make_shared<Transmitter>(colorPort));
  }

  // main loop; send frames
  while (true) {
    steady_clock::time_point t = steady_clock::now();

    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrameTime).count();

    if (dur >= frameDiffTime) {
      lastFrameTime = t;

      auto stream = camInt.getReadyStream();

      if(stream) {

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

        const void* data = stream->getData();
        size_t size = stream->getSize();

        // convert?
        if(converterRef) {
          if(converterRef->convert(data, size)) {
            data = converterRef->getData();
            size = converterRef->getSize();
            // std::cout << "converted to 32bit, size: " << size << std::endl;
          } else {
            data = NULL;
          }
        }

        if(data) {
          if(compressor->compress(data, size)) {
            for(auto t : (*transmitters)) {
              if(t->transmit((const char*)compressor->getData(), compressor->getSize())){
                if(bVerbose) std::cout << "sent " << compressor->getSize() << "-byte " << name << " frame" << std::endl;
              }
            }
          } else {
            std::cout << "FAILED to compress " << depth->getSize() << "-byte " << name << " frame" << std::endl;
          }
        }
      }

      if(clrSrc) {
        (*clrSrc->capRef) >> clrSrc->frame;
        // Mat frame;
        // cap >> frame; // get a new frame from camera
        // cvtColor(frame, edges, CV_BGR2GRAY);
        if(clrSrc->frame.total() > 0)
          std::cout << "Color frame size: " << clrSrc->frame.total() << " with " << clrSrc->frame.channels() << " channels and size: " << clrSrc->frame.size() << std::endl;
      }
    }

    Sleep(sleepTime);

    if(wasKeyboardHit())
      break;
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
