#pragma once

#ifdef OPENNI_AVAILABLE
  #include "OpenNI.h"
#endif

namespace persee {

  class CamInterface {
    public:

    #ifdef OPENNI_AVAILABLE

      typedef ::VideoStream VideoStream

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

  public:

    CamInterface() {
      #ifdef OPENNI_AVAILABLE
      device = getDevice();
        depth = getDepthStream(*device);
        if(depth)
          depth->addNewFrameListener(&depthListener);
        color = getColorStream(*device);
        if(color)
          color->addNewFrameListener(&colorListener);
      #endif
    }

    std::shared_ptr<VideoStream> getDepthStream(){ return depth; }
    std::shared_ptr<VideoStream> getColorStream(){ return color; }
    SimpleFrameListener& getDepthListener(){ return depthListener; }
    SimpleFrameListener& getColorListener(){ return colorListener; }

  private:

    #ifdef OPENNI_AVAILABLE
      std::shared_ptr<Device> device;
    #endif

    SimpleFrameListener depthListener;
    SimpleFrameListener colorListener;
    std::shared_ptr<VideoStream> depth=nullptr, color=nullptr;
  };
}
