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

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <chrono>

// stdlib
#include <netdb.h> //hostent
#include <functional>
#include <math.h>
#include "zlib.h"
// local
#include "../../src/persee/OniSampleUtilities.h"
#include "../../src/persee/CamInterface.h"
#include "../../src/persee/Compressor.h"
#include "../../src/persee/Transmitter.h"

using namespace std;
using namespace std::chrono;
using namespace persee;

int main(int argc, char** argv) {
  // configurables
  bool bResendFrames = false;
  bool bTimed = true;
  float frameDiffTime = 1.0f/1.5f * 1000.0f; // fps
  unsigned int sleepTime = 5; // ms
  int depthPort = argc > 1 ? atoi(argv[1]) : 4445;
  int colorPort = argc > 2 ? atoi(argv[2]) : 4446;

  // attributes
  steady_clock::time_point lastFrameTime = steady_clock::now();

  std::shared_ptr<persee::VideoStream> depth, color;
  persee::CamInterface camInt;
  depth = camInt.createDepthStream();
  color = camInt.createColorStream();

  auto compressor = std::make_shared<Compressor>();

  // Transmitter transmitter(httpPort);
  std::vector<std::shared_ptr<Transmitter>> depthStreamTransmitters;
  std::vector<std::shared_ptr<Transmitter>> colorStreamTransmitters;

  if(depthPort > 0){
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

    // time to send new frame?
    if ( (!bTimed || std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrameTime).count() >= frameDiffTime) ) {
    // do we have data to send?
      if(depth->hasNew() || bResendFrames) {
        depth->update();
        if(compressor->compress(depth->getData(), depth->getSize())) {
          for(auto t : depthStreamTransmitters) {
            t->transmitFrame((const char*)compressor->getData(), compressor->getSize());
            // std::cout << "sent " << compressor->getSize() << "-byte depth frame" << std::endl;
          }
        } else {
          std::cout << "FAILED to compress " << depth->getSize() << "-byte frame" << std::endl;
        }
        depth->reset();
      }

      if(color->hasNew() || bResendFrames) {
        color->update();

        if(compressor->compress(color->getData(), color->getSize())) {
          for(auto t : colorStreamTransmitters) {
            t->transmitFrame((const char*)compressor->getData(), compressor->getSize());
            // std::cout << "sent " << compressor->getSize() << "-byte color frame" << std::endl;
          }
        } else {
          std::cout << "FAILED to compress " << color->getSize() << "-byte frame" << std::endl;
        }

        color->reset();
      }

      lastFrameTime = t;
    }

    Sleep(sleepTime);

    if(wasKeyboardHit())
      break;
  }

  std::cout << "cleaning up..." << std::endl;
  depth->stop();
  depth->destroy();
  color->stop();
  color->destroy();
  camInt.close();

  return 0;
}
