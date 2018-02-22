#pragma once

// of
#include "ofMain.h"
// 3rd party libs
#include "cpp-httplib/httplib.h"
// local livs
#include "ImageStream.h"
#include "persee/Receiver.h"
#include "persee/Inflater.h"

namespace ofxOrbbecPersee {
  class StreamReceiver : public ImageStream::Addon {

    public:

      StreamReceiver(ImageStream* stream, const std::string& addr, int port, char cmd) : ImageStream::Addon(stream), address(addr), port(port), cmd(cmd) {
      }

      void start() {
        // this->requestThread = new std::thread(std::bind(&StreamReceiver::requestThreadFunc, this));
        // ofThread::startThread();

        this->requestReceiverRef = std::make_shared<persee::Receiver>();
        this->requestReceiverRef->setCommFunc([this](persee::Receiver& recvr){
          // request depth stream
          ofLogNotice() << "sending stream request CMD";
          if(!recvr.send_data(&this->cmd, 1)) {
            ofLogWarning() << "Request for stream failed; no connection";
            return true;
          }

          // read response
          ofLogNotice() << "reading stream request response";
          if(!recvr.receive(1)) {
            ofLogWarning() << "Request for stream failed; no response";
            return true;
          }

          // request granted?
          if(recvr.getData()[0] != persee::CMD_OK) {
            ofLogWarning() << "Request for stream failed; get non-OK response: " << recvr.getData()[0] << "(" << (int)recvr.getData()[0] << "," << (int)recvr.getData()[1] << ")";
            return true;
          }

          // read port number from response
          ofLogNotice() << "reading port-number from stream request response";
          if(!recvr.receive(4)) {
            ofLogWarning() << "Request for stream failed; could not get port-number after OK";
            return true;
          }

          // create receiver on the returned port
          int streamPort = recvr.readInt(recvr.getData());
          auto r = std::make_shared<persee::Receiver>(this->address, streamPort);

          recvr.stop();
          this->receiver.start(this->address, streamPort);
          ofLogNotice() << "started receiver on " << this->address << ":" << streamPort << std::endl;
          return false; // don't keep connection alive, we're done
        });

        // start above communication (when connected) in a separate thread
        this->requestReceiverRef->start(address, port);
      }

      void stop(){
        bRequestActive=false;
        receiver.stop();
        inflaterRef.reset();
      }

      virtual void update() {
        // our receiver got data for a new package?
        if(receiver.hasNew()) {
          // "unzip" package to raw frame data
          auto inflater = getInflater();
          if(inflater->inflate(receiver.getFrameData(), receiver.getSize())) {
            // give raw frame data to our ImageStream parent
            this->getImageStream()->offerData((char*)inflater->getData(), inflater->getSize());
          }
        }
      }

    protected:

      virtual void destroy(){
        stop();
      };

      // void threadedFunction() {
      //   while(bRequestActive) {
      //     // ofLogNotice() << "requesting color stream";
      //     try {
      //       httplib::Client cli(address.c_str(), port); // 3 second timeout
      //       ofLogNotice() << "requesting " << address <<":"<<port<<requestPath;
      //
      //       auto res = cli.get(requestPath.c_str());
      //     } catch(exception exc) {
      //       ofLogError() << "exc";
      //     }
      //     //
      //     // ofLogWarning() << "request done";
      //     return;
      //     // if (!res || res->status != 200) {
      //     //   ofLogWarning() << "failed to request color stream";
      //     //   // return false;
      //     //   continue;
      //     // }
      //     //
      //     // int streamport = 0;
      //     //
      //     // try {
      //     //   streamport = std::stoi(res->body);
      //     // } catch(std::invalid_argument exc){
      //     //   ofLogWarning() << exc.what();
      //     //   streamport = 0;
      //     // }
      //     //
      //     // if(streamport < 1){
      //     //   ofLogWarning() << "request for color stream failed";
      //     //   // return false;
      //     //   continue;
      //     // }
      //     //
      //     // ofLogNotice() << "starting color stream receiver";
      //     // receiver.start(address, streamport);
      //     // return; // done
      //   }
      // }

    protected:
      persee::InflaterRef getInflater() {
        if(!this->inflaterRef)
          this->inflaterRef = std::make_shared<persee::Inflater>();
        return this->inflaterRef;
      }

    private:
      std::string address;
      int port; // http port; where to request stream from, NOT the stream port
      char cmd;
      static const int REQUEST_TIMEOUT=5;

      persee::Receiver receiver;
      persee::ReceiverRef requestReceiverRef;
      persee::InflaterRef inflaterRef=nullptr;

      std::thread* requestThread;
      bool bRequestActive=true;
  };
}
