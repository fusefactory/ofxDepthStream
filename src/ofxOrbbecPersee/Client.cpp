// OF
#include "ofMain.h"
// local
#include "Client.h"
#include "StreamReceiver.h"


using namespace ofxOrbbecPersee;
using namespace persee;
//
// void Client::setup(const Client::Options& options) {
//   // receiver.start(options.host, options.port);
//   // ofLogWarning() << "failed to connect to persee at: " << options.host << ":" << options.port << std::endl;
//
//   // // TODO check options for custom format and/or if depthstream is enabled at all
//   // this->depthStreamRef = std::make_shared<DepthStream>();
//   // this->depthStreamRef->setup(640, 480);
//   this->opts = options;
// }

// bool Client::requestStreamComms(Receiver& recvr, char cmd, ImageStreamRef stream){
//   // request depth stream
//   ofLogNotice() << "sending stream request CMD";
//   if(!recvr.send_data(&cmd, 1)) {
//     ofLogWarning() << "Request for stream failed; no connection";
//     return false;
//   }
//
//   // read response
//   ofLogNotice() << "reading stream request response";
//   if(!recvr.receive(1)) {
//     ofLogWarning() << "Request for stream failed; no response";
//     return false;
//   }
//
//   // request granted?
//   if(recvr.getData()[0] != persee::CMD_OK) {
//     ofLogWarning() << "Request for stream failed; get non-OK response: " << recvr.getData()[0] << "(" << (int)recvr.getData()[0] << "," << (int)recvr.getData()[1] << ")";
//     return false;
//   }
//
//   // read port number from response
//   ofLogNotice() << "reading port-number from stream request response";
//   if(!recvr.receive(4)) {
//     ofLogWarning() << "Request for stream failed; could not get port-number after OK";
//     return false;
//   }
//
//   // create receiver on the returned port
//   int streamPort = recvr.readInt(recvr.getData());
//   auto r = std::make_shared<Receiver>(this->opts.host, streamPort);
//   stream->setReceiver(r);
//   ofLogNotice() << "started receiver on " << this->opts.host << ":" << streamPort << std::endl;
//   return true;
// }
//
DepthStreamRef Client::createDepthStream() {
  auto stream = std::make_shared<DepthStream>();
  stream->setup();

  auto streamReceiver = std::make_shared<StreamReceiver>(stream.get(), opts.host, opts.port, "/stream/depth");
  stream->addAddon(streamReceiver);

  streamReceiver->start();
  return stream;
}

ColorStreamRef Client::createColorStream() {
  auto stream = std::make_shared<ColorStream>();
  stream->setup();

  auto streamReceiver = std::make_shared<StreamReceiver>(stream.get(), opts.host, opts.port, "/stream/color");
  stream->addAddon(streamReceiver);

  streamReceiver->start();
  return stream;
}

//
//   //
//   //
//   // // The persee server has one new-incoming-connections port, so we queue any requests
//   // this->performQueuedRequest([this, stream](Receiver& recvr){
//   //   return this->requestStreamComms(recvr, persee::CMD_GET_DEPTH_STREAM, std::static_pointer_cast<ImageStream>(stream));
//   // });
//   //
//   // return stream;
// }
//
// ColorStreamRef Client::createColorStream() {
//   auto stream = std::make_shared<ColorStream>();
//   stream->setup();
//
//   // The persee server has one new-incoming-connections port, so we queue any requests
//   this->performQueuedRequest([this, stream](Receiver& recvr){
//     return this->requestStreamComms(recvr, persee::CMD_GET_COLOR_STREAM, std::static_pointer_cast<ImageStream>(stream));
//   });
//
//   return stream;
// }
//
//
// void Client::performQueuedRequest(QueuedRequestFunc func) {
//   requestQueue.push_back(func);
//   ofLogNotice() << "added item, number in queue: " << this->requestQueue.size();
//   this->processRequestQueue();
// }
//
// void Client::processRequestQueue(){
//   ofLogNotice() << "processRequestQueue";
//
//   if (requestQueue.empty() || bProcessingRequestQueue) return;
//
//   if(receiverRef) // already "running"
//     return;
//
//   receiverRef = std::make_shared<Receiver>();
//
//   receiverRef->setCommFunc([this](Receiver& recvr){
//     while(true) {
//       if(this->requestQueue.size() == 0) {
//         Sleep(REQUEST_IDLE_DELAY);
//         continue;
//       }
//
//       ofLogNotice() << "next queue item, number in queue: " << this->requestQueue.size();
//
//       auto func = this->requestQueue[0];
//
//       if(func(recvr)) {
//         requestAttemptCount=0;
//         requestQueue.erase(requestQueue.begin());
//         ofLogNotice() << "request item done, " << requestQueue.size() << " items left";
//       } else if (requestAttemptCount >= MAX_REQUEST_ATTEMPTS) {
//         ofLogError() << "request failed";
//         requestAttemptCount=0;
//         requestQueue.erase(requestQueue.begin());
//       } else {
//         ofLogNotice() << "retrying request queue item after short delay...";
//         requestAttemptCount+=1;
//         Sleep(REQUEST_RETRY_DELAY);
//       }
//     } // while true
//
//   });
//
//   // todo; make it stop when nothing to do?
//   receiverRef->start(opts.host, opts.port);
// }
//
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
