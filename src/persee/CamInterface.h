#pragma once

#include <memory>
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

    class SimpleFrameListener : public openni::VideoStream::NewFrameListener {
      public:
        void onNewFrame(openni::VideoStream& stream) {
          bHasNew = true;
        }

        bool hasNew() const { return bHasNew; }
        void reset() { bHasNew = false; }

      private:
        bool bHasNew = false;
    };

  #else

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
          auto s = getDepthStream(*device);
          return std::make_shared<VideoStream>(s);
        #else
          return std::make_shared<VideoStream>(std::make_shared<openni::VideoStream>());
        #endif
      }

      VideoStreamRef createColorStream() {
        #ifdef OPENNI_AVAILABLE
          if(!device) device = getDevice();
          auto s = getColorStream(*device);
          return std::make_shared<VideoStream>(s);
        #else
          return std::make_shared<VideoStream>(std::make_shared<openni::VideoStream>());
        #endif
      }

      void close(){
        #ifdef OPENNI_AVAILABLE
          if(device) device->close();
          openni::OpenNI::shutdown();
        #endif
      }

    private:

      #ifdef OPENNI_AVAILABLE
        std::shared_ptr<openni::Device> getDevice();
        std::shared_ptr<openni::VideoStream> getDepthStream(openni::Device& device);
        std::shared_ptr<openni::VideoStream> getColorStream(openni::Device& device);

        std::shared_ptr<openni::Device> device;
      #endif
  };
}
