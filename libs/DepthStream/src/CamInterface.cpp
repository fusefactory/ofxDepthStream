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

#include <iostream>
#include <math.h>
#include <math.h>
#include "CamInterface.h"

using namespace openni;
using namespace depth;

#ifdef OPENNI_AVAILABLE

std::shared_ptr<openni::Device> CamInterface::getDevice() {
  Status rc = OpenNI::initialize();

  if (rc != STATUS_OK) {
    printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
    return nullptr;
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

  auto device = std::make_shared<openni::Device>();
  rc = device->open(ANY_DEVICE);
  if (rc != STATUS_OK) {
    printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
    return nullptr;
  }

  return device;
}

std::shared_ptr<openni::VideoStream> CamInterface::getDepthStream(openni::Device& device) {
  auto stream = std::make_shared<openni::VideoStream>();
  Status rc;

  if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
  {
    rc = stream->create(device, SENSOR_DEPTH);
    if (rc != STATUS_OK)
    {
      printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }

    rc = stream->start();
    if (rc != STATUS_OK)
    {
      printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }
  }

  return stream;
}

std::shared_ptr<openni::VideoStream> CamInterface::getColorStream(openni::Device& device) {
  auto stream = std::make_shared<openni::VideoStream>();
  Status rc;
  if (device.getSensorInfo(SENSOR_COLOR) != NULL)
  {
    rc = stream->create(device, SENSOR_COLOR);
    if (rc != STATUS_OK)
    {
      printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }

    rc = stream->start();
    if (rc != STATUS_OK)
    {
      printf("Couldn't start the color stream\n%s\n", OpenNI::getExtendedError());
      return nullptr;
    }
  }

  return stream;
}

void Formatter::process(openni::VideoStream& stream) {
  VideoFrameRef frame;
  stream.readFrame(&frame);

  int pixelCount = frame.getHeight()*frame.getWidth();
  size_t pixelSize;
  switch (frame.getVideoMode().getPixelFormat())
  {
    case PIXEL_FORMAT_DEPTH_1_MM: // <-- this one is begin given by the persee by default
    case PIXEL_FORMAT_DEPTH_100_UM: {
      pixelSize = sizeof(DepthPixel);
      this->size = pixelCount * pixelSize;
      this->data = (const unsigned char*)frame.getData();
      // memcpy(buffer, frame.getData(), this->size);
      // std::cout << "formatter: found " << this->size << " depth bytes" << std::endl;
      break;
    }

    case PIXEL_FORMAT_RGB888: {
      pixelSize = sizeof(RGB888Pixel);
      this->size = pixelCount * pixelSize;
      this->data = (const unsigned char*)frame.getData();
      // memcpy(buffer, frame.getData(), this->size);
      // std::cout << "formatter: found " << this->size << " color bytes" << std::endl;
      break;
    }

    default:
      std::cerr << "unsupported pixel format" << std::endl;
  }
}

#else

// Dummy implementation which provides a
void Formatter::process(openni::VideoStream& stream) {
  this->size = BUF_SIZE;
  for(int i=0; i<size; i+=2){
    int v = (sinf(i*0.1f) + 1.0f) / 2.0f * ((1 << 15)-1);

    this->data[i+1] = (v & 0xff) << 8;
    this->data[i] = (v & 0xff);
  }
}

#endif


VideoStreamRef CamInterface::getReadyStream(){
  #ifdef OPENNI_AVAILABLE
  openni::VideoStream* streams[] = {depth->getStream().get(), color->getStream().get()};

  int readyStream = -1;
  Status rc = OpenNI::waitForAnyStream(streams, 2, &readyStream, SAMPLE_READ_WAIT_TIMEOUT);
  if (rc != STATUS_OK) {
    std::cerr << "Wait failed! " << OpenNI::getExtendedError() << std::endl;
    return nullptr;
  }

  switch (readyStream)
  {
  case 0:
    // Depth
    return depth;
    break;
  case 1:
    // Color
    return color;
    break;
  default:
    std::cerr << "Unxpected stream" << std::endl;
  }

  return nullptr;
  #else
    flipFlop = !flipFlop;
    return flipFlop ? depth : color;
  #endif
}
