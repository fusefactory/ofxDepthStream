#pragma once

// stdlib
#include "zlib.h"
// of
#include "ofMain.h"
// ofxaddons
// #include "ofxTCPClient.h"
// local livs
#include "ImageStream.h"
#include "persee/Receiver.h"
#include "persee/Inflater.h"
#include "persee/protocol.h"

namespace ofxOrbbecPersee {
  class StreamReceiver : public ImageStream::Addon, public ofThread {

    public:

      StreamReceiver(ImageStream* stream, const std::string& addr, int port) : ImageStream::Addon(stream), address(addr), port(port) {
      }

      void start() {
        receiver.start(address, port);
      }

      void stop() {
        receiver.stop();
      }

      virtual void update() {
        if(!receiver.hasNew())
          return;

        this->process(receiver.getData(), receiver.getSize());

        // sets internal "hasNew" flag to false
        receiver.reset();
      }

      persee::Receiver& getReceiver() { return receiver; }

      void process(void* data, size_t size){
        // inflate ("unzip") received data
        if(!inflater.inflate((char*)data, size)) {
          ofLogWarning() << "Inflation of " << size << "-byte package FAILED";
          return;
        }

        auto packData = inflater.getData();
        auto packSize = inflater.getSize();
        // std::cout << "Got " << packSize << "-byte package, inflated to " << packSize << "-byte frame" << std::endl;

        // if(packSize == DepthStream::FRAME_SIZE_640x480x16BIT) {
          getImageStream()->offerData((void*)packData, packSize);
        // } else {

        // }

      }
    protected:

      virtual void destroy(){
        this->stop();
      };

    private:
      std::string address;
      int port; // http port; where to request stream from, NOT the stream port

      persee::Receiver receiver;
      persee::Inflater inflater;
  };
}
