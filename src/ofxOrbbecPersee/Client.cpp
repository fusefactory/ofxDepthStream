// OF
#include "ofMain.h"
// local
#include "persee/protocol.h"
#include "Client.h"
#include "StreamReceiver.h"


using namespace ofxOrbbecPersee;
using namespace persee;

DepthStreamRef Client::createDepthStream() {
  // create image stream; in charge of converting a batch of pixel-data into a texture
  auto stream = std::make_shared<DepthStream>();
  stream->setup();

  // create the stream's client addon
  auto streamReceiver = std::make_shared<StreamReceiver>(stream.get(), opts.host, opts.depthStreamPort);
  stream->addAddon(streamReceiver);
  streamReceiver->start();

  return stream;
}

ColorStreamRef Client::createColorStream() {
  // create image stream; in charge of converting a batch of pixel-data into a texture
  auto stream = std::make_shared<ColorStream>();
  stream->setup();

  // create the stream's client addon
  auto streamReceiver = std::make_shared<StreamReceiver>(stream.get(), opts.host, opts.colorStreamPort);
  stream->addAddon(streamReceiver);
  streamReceiver->start();

  return stream;
}
