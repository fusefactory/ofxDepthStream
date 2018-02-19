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
#include <chrono>

// stdlib
#include<netdb.h> //hostent
#include <functional>
#include <math.h>
#include "zlib.h"
// local
#include "../../src/persee/OniSampleUtilities.h"
#include "../../src/persee/Formatter.h"
#include "../../src/persee/Compressor.h"
#include "../../src/persee/Transmitter.h"

using namespace std;
using namespace std::chrono;
using namespace persee;

#ifdef OPENNI_AVAILABLE
  #include "OpenNI.h"

  using namespace openni;

  class SimpleFrameListener : public VideoStream::NewFrameListener {
    public:
      void onNewFrame(VideoStream& stream) {
        bHasNew = true;
      }

      bool hasNew() const { return bHasNew; }
      void reset() { bHasNew = false; }

    private:
      bool bHasNew = false;
  };

#else

  typedef int VideoStream;

  // dummy placeholder which always has a new frame for us
  class SimpleFrameListener {
    public:
      bool hasNew() const { return true; }
      void reset() {  }
  };

#endif

#ifdef OPENNI_AVAILABLE
  int setup(Device& device, VideoStream& depth) {
    Status rc = OpenNI::initialize();

    if (rc != STATUS_OK)
    {
      printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
      return 1;
    }

    // OpenNIDeviceListener devicePrinter;
    //
    // OpenNI::addDeviceConnectedListener(&devicePrinter);
    // OpenNI::addDeviceDisconnectedListener(&devicePrinter);
    // OpenNI::addDeviceStateChangedListener(&devicePrinter);

    // openni::Array<openni::DeviceInfo> deviceList;
    // openni::OpenNI::enumerateDevices(&deviceList);
    // for (int i = 0; i < deviceList.getSize(); ++i)
    // {
    //   printf("Device \"%s\" already connected\n", deviceList[i].getUri());
    // }


    rc = device.open(ANY_DEVICE);
    if (rc != STATUS_OK)
    {
      printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
      return 2;
    }

    if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
    {
      rc = depth.create(device, SENSOR_DEPTH);
      if (rc != STATUS_OK)
      {
        printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
      }
    }
    rc = depth.start();
    if (rc != STATUS_OK)
    {
      printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
    }

    return 0;
  }
#endif

int main(int argc, char** argv) {
  // configurables
  bool bResendFrames = false;
  bool bTimed = true;
  float frameDiffTime = 1.0f/1.5f * 1000.0f; // fps
  unsigned int sleepTime = 5; // ms
  int httpPort = 4444;

  // attributes
  steady_clock::time_point lastFrameTime = steady_clock::now();
  #ifdef OPENNI_AVAILABLE
  Device device;
  #endif
  VideoStream depth;
  SimpleFrameListener listener;
  Formatter formatter;
  auto compressor = std::make_shared<Compressor>();
  Transmitter transmitter(httpPort);

  // Formatter

  #ifdef OPENNI_AVAILABLE
  { // setup
    int result = setup(device, depth);

    if (result != 0)
      return result;

    // Register to new frame
    depth.addNewFrameListener(&listener);
  }
  #endif

  // main loop; send frames
  while (!wasKeyboardHit())
  {
    steady_clock::time_point t = steady_clock::now();

    // time to send new frame?
    if (  (!bTimed || std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrameTime).count() >= frameDiffTime)
    // do we have data to send?
      &&  (listener.hasNew() || bResendFrames)) {
      formatter.process(depth);

      if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
        if(transmitter.transmit(compressor->getData(), compressor->getSize())) {
          std::cout << "sent compressed " << formatter.getSize() << "-byte frame in " << compressor->getSize() << "-byte package" << std::endl;
        } else {
          if (transmitter.hasClient()) {
            std::cout << "FAILED to send compressed " << formatter.getSize() << "-byte frame in " << compressor->getSize() << "-byte package" << std::endl;
          } else {
            std::cout << "No receiver for compressed " << formatter.getSize() << "-byte frame in " << compressor->getSize() << "-byte package" << std::endl;
          }
        }
      } else {
        std::cout << "FAILED to compress " << formatter.getSize() << "-byte frame" << std::endl;
      }

      listener.reset();
      lastFrameTime = t;
    }

    Sleep(sleepTime);
  }

  #ifdef OPENNI_AVAILABLE
  { // cleanup
    depth.stop();
    depth.destroy();
    device.close();
    OpenNI::shutdown();
  }
  #endif

  return 0;
}
