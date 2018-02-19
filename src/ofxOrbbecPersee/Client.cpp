#include "ofMain.h"
#include "Client.h"

using namespace ofxOrbbecPersee;

void Client::setup(const Client::Options& options) {
  receiver.start(options.host, options.port);
  // ofLogWarning() << "failed to connect to persee at: " << options.host << ":" << options.port << std::endl;

  // TODO check options for custom format and/or if depthstream is enabled at all
  this->depthStreamRef = std::make_shared<DepthStream>();
  this->depthStreamRef->setup(640, 480);
}

void Client::update() {
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

  if(packSize == DepthStream::FRAME_SIZE_640x480x16BIT) {
    this->getDepthStream().update(packData, packSize);
  }

  // sets internal "hasNew" flag to false
  receiver.reset();
}
