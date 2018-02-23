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
        // TODO
        ofLogNotice() << "Stream receiver stom";
        receiver.stop();
      }

      virtual void update() {
        if(!receiver.hasNew())
          return;

        // inflate ("unzip") received data
        if(!inflater.inflate(receiver.getData(), receiver.getSize())) {
          ofLogWarning() << "Inflation of " << receiver.getSize() << "-byte package FAILED";
          return;
        }

        auto packData = inflater.getData();
        auto packSize = inflater.getSize();
        // std::cout << "Got " << packSize << "-byte package, inflated to " << packSize << "-byte frame" << std::endl;

        // if(packSize == DepthStream::FRAME_SIZE_640x480x16BIT) {
          getImageStream()->offerData((void*)packData, packSize);
        // } else {

        // }

        // sets internal "hasNew" flag to false
        receiver.reset();
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
