#include "ofApp.h"

void ofApp::setup() {
  ofSetWindowShape(1280,480);
  // create tcp network receivers for both the depth and the color stream
  depthReceiverRef = depth::Receiver::createAndStart(remoteCamAddress, depthPort);
  // colorReceiverRef = depth::Receiver::createAndStart(remoteCamAddress, colorPort); // color stream isn't working yet on the transmitter side...
  inflaterRef = std::make_shared<depth::Inflater>(1280 * 720 * 2 /* initial buffer size, optional */);
}

void ofApp::update() {
  // checks if our receivers have new data, if so these convenience methods
  // update (and allocate if necessary!) our textures.
  ofxDepthStream::loadDepthTexture(*depthReceiverRef, depthTex,
    ofxDepthStream::DepthLoaderOpts().useInflater(this->inflaterRef));
  // ofxDepthStream::loadColorTexture(*colorReceiverRef, colorTex);
}

void ofApp::draw() {
  ofBackground(0);

  if(depthTex.isAllocated()) {
    depthTex.draw(0, 0);
  }

  // if(colorTex.isAllocated()) {
  //   colorTex.draw(640, 0);
  // }
}
