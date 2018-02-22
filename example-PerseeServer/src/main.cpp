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
// 3rd party libs
#include "cpp-httplib/httplib.h"
// local
#include "../../src/persee/OniSampleUtilities.h"
#include "../../src/persee/Formatter.h"
#include "../../src/persee/Compressor.h"
#include "../../src/persee/Transmitter.h"
#include "../../src/persee/protocol.h"

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

  std::shared_ptr<Device> getDevice() {
    Status rc = OpenNI::initialize();

    if (rc != STATUS_OK) {
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

    auto device = std::make_shared<Device>();
    rc = device->open(ANY_DEVICE);
    if (rc != STATUS_OK) {
      printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }

    rc = depth->start();
    if (rc != STATUS_OK)
    {
      printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }

    return device;
  }

  std::shared_ptr<VideoStream> getDepthStream(Device& device) {
    auto stream = std::make_shared<VideoStream>();

    if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
    {
      rc = stream->create(device, SENSOR_DEPTH);
      if (rc != STATUS_OK)
      {
        printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
        return nullptr;
      }
    }

    return stream;
  }

  std::shared_ptr<VideoStream> getColorStream(Device& device) {
    auto stream = std::make_shared<VideoStream>();

    if (device.getSensorInfo(SENSOR_COLOR) != NULL)
    {
      rc = stream->create(device, SENSOR_COLOR);
      if (rc != STATUS_OK)
      {
        printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
        return nullptr;
      }
    }

    return stream;
  }

#else

  typedef int VideoStream;

  // dummy placeholder which always has a new frame for us
  class SimpleFrameListener {
    public:
      bool hasNew() const { return true; }
      void reset() {  }
  };

#endif


class MainApp {
  private:

    // configurables
    bool bResendFrames = false;
    bool bTimed = true;
    float frameDiffTime = 1.0f/1.5f * 1000.0f; // fps
    unsigned int sleepTime = 5; // ms
    int httpPort;

    // attributes
    steady_clock::time_point lastFrameTime;
    Formatter formatter;
    std::shared_ptr<Compressor> compressor;
    int lastPort; // for every new connection we'll start a new transmitter at a new port
    httplib::Server httpServer;

    // Transmitter transmitter(httpPort);
    #ifdef OPENNI_AVAILABLE
      std::shared_ptr<Device> device;
    #endif

    std::shared_ptr<VideoStream> depth, color;
    SimpleFrameListener depthListener;
    SimpleFrameListener colorListener;
    std::vector<std::shared_ptr<Transmitter>> depthStreamTransmitters;
    std::vector<std::shared_ptr<Transmitter>> colorStreamTransmitters;

  public:

    MainApp(int httpPort) : httpPort(httpPort) {
      compressor = std::make_shared<Compressor>();

      #ifdef OPENNI_AVAILABLE
        std::shared_ptr<Device> device = getDevice();
      #endif

      httpServer.get("/stream/depth", [](const httplib::Request& req, httplib::Response& res) {
        // res.set_content("Hello World!", "text/plain");
      });

      httpServer.get("/stream/color", [&](const httplib::Request& req, httplib::Response& res) {
        this->onColorStreamRequest(res);
        // auto numbers = req.matches[1];
        // res.set_content(numbers, "text/plain");
      });

      httpServer.listen("localhost", httpPort);

      lastFrameTime = steady_clock::now();
      lastPort = httpPort;
    }

    ~MainApp() {
      #ifdef OPENNI_AVAILABLE
        stopColorStream();
        stopDepthStream();
        device->close();
        OpenNI::shutdown();
      #endif
    }

    bool update() {
      steady_clock::time_point t = steady_clock::now();

      // time to send new frame?
      if ( (!bTimed || std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrameTime).count() >= frameDiffTime) ) {
      // do we have data to send?
        if(depthListener.hasNew() || bResendFrames) {
          formatter.process(*depth);

          if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
            for(auto t : depthStreamTransmitters) {
              t->transmitFrame((const char*)compressor->getData(), compressor->getSize());
              std::cout << "sent " << compressor->getSize() << "-byte depth frame" << std::endl;
            }
          } else {
            std::cout << "FAILED to compress " << formatter.getSize() << "-byte frame" << std::endl;
          }

          depthListener.reset();
        }

        if(colorListener.hasNew() || bResendFrames) {
          formatter.process(*depth);

          if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
            for(auto t : colorStreamTransmitters) {
              t->transmitFrame((const char*)compressor->getData(), compressor->getSize());
              std::cout << "sent " << compressor->getSize() << "-byte color frame" << std::endl;
            }
          } else {
            std::cout << "FAILED to compress " << formatter.getSize() << "-byte frame" << std::endl;
          }

          colorListener.reset();
        }

        lastFrameTime = t;
      }

      Sleep(sleepTime);
      return true; // never stop!
    }

  private:

    void startColorStream(){
      #ifdef OPENNI_AVAILABLE
        stopColorStream();
        if(!device) device = getDevice();
        if(device) color = getColorStream(*device);
        if(color) color->addNewFrameListener(&depthListener);
      #else
        depth = std::make_shared<VideoStream>(); // dev-only dummy
      #endif

      std::cout << "started camera color stream" << std::endl;
    }

    void startDepthStream(){
      #ifdef OPENNI_AVAILABLE
        stopDepthStream();
        if(!device) device = getDevice();
        if(device) depth = getDepthStream(*device);
        if(depth) depth->addNewFrameListener(&depthListener);
      #else
        depth = std::make_shared<VideoStream>(); // dev-only dummy
      #endif
      std::cout << "started camera depth stream" << std::endl;
    }

    void stopColorStream(){

      if(color){
        #ifdef OPENNI_AVAILABLE
        color->stop()
        color->destroy();
        #endif
        color = nullptr;

        std::cout << "stopped camera color stream" << std::endl;
      }
    }

    void stopDepthStream(){
      if(depth){
        #ifdef OPENNI_AVAILABLE
        depth->stop()
        depth->destroy();
        #endif
        depth = nullptr;

        std::cout << "stopped camera depth stream" << std::endl;
      }
    }

    void onColorStreamRequest(httplib::Response& res) {
      auto transmitter = std::make_shared<Transmitter>(startPort);

      // when bind fails; retry with new port
      transmitter->setBindFailedHandler([](Transmitter& t){
        // try next port
        t.setPort(t.getPort()+1);
      });

      // if(!transmitter){
      //   std::cerr << "Failed to create color-stream transmitter" << std::endl;
      //   res.set_content(std::to_string(CMD_ERROR), "text/plain");
      //   // t.transmitRaw((const char*)&response, 1);
      //   return;
      // }

      transmitter->whenBound([this, &res, transmitter](Transmitter& newtransmitter){
        // std::cout << "new depth-stream transmitter started" << std::endl;
        // save our new transmitter
        if(colorStreamTransmitters.size() == 0)
          startColorStream();

        colorStreamTransmitters.push_back(transmitter);

        // update lastPort for creation of next transmitter
        lastPort = transmitter->getPort();

        // respond with OK-byte and the port number (4-byte integer)
        res.set_content(std::to_string(CMD_OK)+std::to_string(transmitter->getPort()), "text/plain");
        // char response = CMD_OK;
        // t.transmitRaw((const char*)&response, 1);
        // t.transmitInt(transmitter->getPort());
        std::cout << "Started new Color stream on port: " << transmitter->getPort() << std::endl;

        transmitter->whenUnbound([this, &res, transmitter](Transmitter& tt){
          // don't reconnect
          tt.setBoundHandler(nullptr);
          tt.stop();
          // std::cout << "Transmitter unbound from port: " << transmitter->getPort() << std::endl;

          for(auto it=colorStreamTransmitters.begin(); it != colorStreamTransmitters.end(); it++) {
            if(*it == transmitter) {
              // std::cout << "removed transmitter from depth stream list" << std::endl;
              colorStreamTransmitters.erase(it);

              if(colorStreamTransmitters.size() == 0)
                stopColorStream();

              break;
            }
          }

          std::cout << "Cleaned up color-stream transmitter from port " << transmitter->getPort() << std::endl;
        });
      });
    }
};

int main(int argc, char** argv) {
  int httpPort = argc > 1 ? atoi(argv[1]) : 4444;

  MainApp app(httpPort);

  while(app.update()) {
    if(wasKeyboardHit())
      break;
  }

  return 0;

  // // configurables
  // bool bResendFrames = false;
  // bool bTimed = true;
  // float frameDiffTime = 1.0f/1.5f * 1000.0f; // fps
  // unsigned int sleepTime = 5; // ms
  // int httpPort = argc > 1 ? atoi(argv[1]) : 4444;
  //
  // // attributes
  // steady_clock::time_point lastFrameTime = steady_clock::now();
  // std::shared_ptr<VideoStream> depth, color;
  // SimpleFrameListener depthListener;
  // SimpleFrameListener colorListener;
  // Formatter formatter;
  // auto compressor = std::make_shared<Compressor>();
  //
  // #ifdef OPENNI_AVAILABLE
  //   std::shared_ptr<Device> device = getDevice();
  //   depth = getDepthStream(*device);
  //   if(depth)
  //     depth->addNewFrameListener(&depthListener);
  //   color = getColorStream(*device);
  //   if(color)
  //     color->addNewFrameListener(&colorListener);
  // #endif


  // // Transmitter transmitter(httpPort);
  // std::vector<std::shared_ptr<Transmitter>> depthStreamTransmitters;
  // std::vector<std::shared_ptr<Transmitter>> colorStreamTransmitters;
  //
  // int lastPort = httpPort; // for every new connection we'll start a new transmitter at a new port
  //
  //
  // httplib::Server httpServer;
  //
  // httpServer.get("/stream/depth", [](const httplib::Request& req, httplib::Response& res) {
  //   // res.set_content("Hello World!", "text/plain");
  // });
  //
  // httpServer.get("/stream/color", [&](const httplib::Request& req, httplib::Response& res) {
  //   // auto numbers = req.matches[1];
  //   // res.set_content(numbers, "text/plain");
  // });
  //
  // httpServer.listen("localhost", httpPort);
  //
  //
  // bool bKeepGoing = true;


  // Transmitter newConnectionTransmitter(httpPort);
  // newConnectionTransmitter.setFirstByteHandler(
  //   [&lastPort, &depthStreamTransmitters, &colorStreamTransmitters](Transmitter& t, char byte){
  //
  //   // remember this method is executed on the newConnectionTransmitter's listener thread
  //
  //   switch(byte) {
  //     case CMD_GET_DEPTH_STREAM: {
  //       // std::cout << "Got CMD_GET_DEPTH_STREAM" << std::endl;
  //
  //       auto transmitter = createTransmitter(lastPort+1);
  //
  //       if(!transmitter){
  //         std::cerr << "Failed to create depth-stream transmitter" << std::endl;
  //         char response = CMD_ERROR;
  //         t.transmitRaw((const char*)&response, 1);
  //         return;
  //       }
  //
  //       transmitter->whenBound([
  //         &t, &lastPort, &depthStreamTransmitters, transmitter](Transmitter& newtransmitter){
  //         // std::cout << "new depth-stream transmitter started" << std::endl;
  //         // save our new transmitter
  //         depthStreamTransmitters.push_back(transmitter);
  //         // update lastPort for creation of next transmitter
  //         lastPort = transmitter->getPort();
  //
  //         // respond with OK-byte and the port number (4-byte integer)
  //         char response = CMD_OK;
  //         t.transmitRaw((const char*)&response, 1);
  //         t.transmitInt(transmitter->getPort());
  //         std::cout << "Started new Depth stream on port: " << transmitter->getPort() << std::endl;
  //
  //         transmitter->whenUnbound([&depthStreamTransmitters, transmitter](Transmitter& tt){
  //           // don't reconnect
  //           tt.setBoundHandler(nullptr);
  //           tt.stop();
  //           // std::cout << "Transmitter unbound from port: " << transmitter->getPort() << std::endl;
  //
  //           for(auto it=depthStreamTransmitters.begin(); it != depthStreamTransmitters.end(); it++) {
  //             if(*it == transmitter) {
  //               // std::cout << "removed transmitter from depth stream list" << std::endl;
  //               depthStreamTransmitters.erase(it);
  //               break;
  //             }
  //           }
  //
  //           std::cout << "Cleaned up depth-stream transmitter from port " << transmitter->getPort() << std::endl;
  //         });
  //       });
  //
  //       return;
  //     }
  //
  //     case CMD_GET_COLOR_STREAM: {
  //       // std::cout << "Got CMD_GET_DEPTH_STREAM" << std::endl;
  //
  //       auto transmitter = createTransmitter(lastPort+1);
  //
  //       if(!transmitter){
  //         std::cerr << "Failed to create color-stream transmitter" << std::endl;
  //         char response = CMD_ERROR;
  //         t.transmitRaw((const char*)&response, 1);
  //         return;
  //       }
  //
  //       transmitter->whenBound([
  //         &t, &lastPort, &colorStreamTransmitters, transmitter](Transmitter& newtransmitter){
  //         // std::cout << "new depth-stream transmitter started" << std::endl;
  //         // save our new transmitter
  //         colorStreamTransmitters.push_back(transmitter);
  //         // update lastPort for creation of next transmitter
  //         lastPort = transmitter->getPort();
  //
  //         // respond with OK-byte and the port number (4-byte integer)
  //         char response = CMD_OK;
  //         t.transmitRaw((const char*)&response, 1);
  //         t.transmitInt(transmitter->getPort());
  //         std::cout << "Started new Color stream on port: " << transmitter->getPort() << std::endl;
  //
  //         transmitter->whenUnbound([&colorStreamTransmitters, transmitter](Transmitter& tt){
  //           // don't reconnect
  //           tt.setBoundHandler(nullptr);
  //           tt.stop();
  //           // std::cout << "Transmitter unbound from port: " << transmitter->getPort() << std::endl;
  //
  //           for(auto it=colorStreamTransmitters.begin(); it != colorStreamTransmitters.end(); it++) {
  //             if(*it == transmitter) {
  //               // std::cout << "removed transmitter from depth stream list" << std::endl;
  //               colorStreamTransmitters.erase(it);
  //               break;
  //             }
  //           }
  //
  //           std::cout << "Cleaned up color-stream transmitter from port " << transmitter->getPort() << std::endl;
  //         });
  //       });
  //
  //       return;
  //     }
  //   }
  // });
}
