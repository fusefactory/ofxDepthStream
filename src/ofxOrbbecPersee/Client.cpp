// OF
#include "ofMain.h"
// local
#include "Client.h"
#include "persee/Receiver.h"
#include "persee/protocol.h"

using namespace ofxOrbbecPersee;
using namespace persee;

void Client::setup(const Client::Options& options) {
  // receiver.start(options.host, options.port);
  // ofLogWarning() << "failed to connect to persee at: " << options.host << ":" << options.port << std::endl;

  // // TODO check options for custom format and/or if depthstream is enabled at all
  // this->depthStreamRef = std::make_shared<DepthStream>();
  // this->depthStreamRef->setup(640, 480);
  this->opts = options;
}

DepthStreamRef Client::createDepthStream() {
  auto stream = std::make_shared<DepthStream>();

  // The persee server has one new-incoming-connections port, so we queue any requests
  this->performQueuedRequest([this, stream](Receiver& recvr){
    // request depth stream
    char cmd = persee::CMD_GET_DEPTH_STREAM;
    if(recvr.send_data(&cmd, 1)) {
      if(recvr.receive(1)) {
        // request granted?
        if(*recvr.getData() == persee::CMD_OK) {
          if(recvr.receive(4)) {
            // create receiver on the returned port
            int streamPort = recvr.readInt(recvr.getData());
            auto r = std::make_shared<Receiver>(this->opts.host, streamPort);
            stream->setReceiver(r);
          }
        }
      }
    }

    // deprecate this lambda to avoid memory leak? (TODO does this work??)
    recvr.setCommFunc(nullptr);
    recvr.stop();
  });

  return stream;
}

void Client::performQueuedRequest(QueuedRequestFunc func) {
  requestQueue.push_back(func);
}

void Client::processRequestQueue(){
  if (requestQueue.empty() || bProcessingRequestQueue) return;
  bProcessingRequestQueue = true;
  this->processRequestQueueItem(requestQueue[0]);
  requestQueue.erase(requestQueue.begin());
  // bProcessingRequestQueue = false;
}

void Client::processRequestQueueItem(QueuedRequestFunc func){
  // create receiver which connects to server
  // requests a stream connection, and when confirmed,
  // creates a separate stream receiver
  if(!receiverRef) {
    receiverRef = std::make_shared<Receiver>();

    receiverRef->setIdleFunc([this](Receiver& recvr){
      this->bProcessingRequestQueue = false;
      this->processRequestQueue();
    });
  }

  receiverRef->setCommFunc([this, func](Receiver& recvr){
    func(recvr);
    recvr.stop();
  });

  receiverRef->start(this->opts.host, this->opts.port);

  // Receiver receiver;
  // receiver.setCommFunc(func);
  // receiver.start(this->opts.host, this->opts.port);
}

// void Client::update() {
//   if(!receiver.hasNew())
//     return;
//
//   // inflate ("unzip") received data
//   if(!inflater.inflate(receiver.getData(), receiver.getSize())) {
//     ofLogWarning() << "Inflation of " << receiver.getSize() << "-byte package FAILED";
//     return;
//   }
//
//   auto packData = inflater.getData();
//   auto packSize = inflater.getSize();
//   // std::cout << "Got " << packSize << "-byte package, inflated to " << packSize << "-byte frame" << std::endl;
//
//   if(packSize == DepthStream::FRAME_SIZE_640x480x16BIT) {
//     this->getDepthStream().update(packData, packSize);
//   }
//
//   // sets internal "hasNew" flag to false
//   receiver.reset();
// }
