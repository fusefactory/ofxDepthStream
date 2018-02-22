#pragma once

#include "config.h"

#ifdef OPENNI_AVAILABLE
  #include "OpenNI.h"
#else
  // create "dummy" openni::VideoStream type
  namespace openni {
    using VideoStream = int;
  }
#endif


namespace persee {

  class Formatter {
    public:
      void process(openni::VideoStream& stream);
      const char* getData() const { return data; }
      int getSize(){ return size; }

    private:

      #ifdef OPENNI_AVAILABLE
        const char* data = NULL;
      #else
        static const size_t BUF_SIZE = 640*480*2;
        char data[BUF_SIZE];
      #endif
      unsigned int size=0;
  };

  #ifdef OPENNI_AVAILABLE

    class SimpleFrameListener : public VideoStream::NewFrameListener {
      public:
        void onNewFrame(openni::VideoStream& stream) {
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

    std::shared_ptr<openni::VideoStream> getDepthStream(Device& device) {
      auto stream = std::make_shared<openni::VideoStream>();

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

    std::shared_ptr<openni::VideoStream> getColorStream(Device& device) {
      auto stream = std::make_shared<openni::VideoStream>();

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

    // typedef int VideoStream;

    // dummy placeholder which always has a new frame for us
    class SimpleFrameListener {
      public:
        bool hasNew() const { return true; }
        void reset() {  }
    };

  #endif


  class VideoStream;
  typedef std::shared_ptr<VideoStream> VideoStreamRef;
  class VideoStream {
    public:
      VideoStream(std::shared_ptr<openni::VideoStream> streamRef) : streamRef(streamRef){
        #ifdef OPENNI_AVAILABLE
        streamRef->addNewFrameListener(&listener);
        #endif
      }

      bool hasNew() const { return listener.hasNew(); };
      void reset(){ listener.reset(); }
      void update(){ formatter.process(*streamRef); }
      const char* getData(){ return (char*)formatter.getData(); }
      int getSize(){ return formatter.getSize(); }

      void stop(){
        #ifdef OPENNI_AVAILABLE
        streamRef->stop();
        #endif
      }

      void destroy(){
        #ifdef OPENNI_AVAILABLE
        streamRef->destroy();
        #endif
      }

    private:
      Formatter formatter;
      std::shared_ptr<openni::VideoStream> streamRef;
      SimpleFrameListener listener;
  };

  /**
   * The Whole purpose of this wrapper is to "hide" all OpenNI related stuff and replace
   * it with placeholder code for the purpose of development. Make sure that OPENNI_AVAILABLE
   * is defined to use the real camera!
   */
  class CamInterface {

    public:

      VideoStreamRef createDepthStream() {
        #ifdef OPENNI_AVAILABLE
          if(!device) device = getDevice();
          auto s = getDepthStream(device);
          return std::make_shared<VideoStream>(s);
        #else
          return std::make_shared<VideoStream>(std::make_shared<openni::VideoStream>());
        #endif
      }

      VideoStreamRef createColorStream() {
        #ifdef OPENNI_AVAILABLE
          if(!device) device = getDevice();
          auto s = getColorStream(device);
          return std::make_shared<VideoStream>(s);
        #else
          return std::make_shared<VideoStream>(std::make_shared<openni::VideoStream>());
        #endif
      }

      void close(){
        #ifdef OPENNI_AVAILABLE
          if(device) device->close();
          OpenNI::shutdown();
        #endif
      }

    private:

      #ifdef OPENNI_AVAILABLE
        std::shared_ptr<Device> device;
      #endif
  };
}
