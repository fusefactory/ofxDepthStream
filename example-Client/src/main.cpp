//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

// OF
#include "ofMain.h"
// addons
#include "ofxDepthStream/ofxDepthStream.h"

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

  private: // attributes
    std::string remoteCamAddress = "127.0.0.1"; // persee.local"; //"192.168.1.226"; // "127.0.0.1";
    int depthPort = 4445;
    // int colorPort = 4446;

    depth::ReceiverRef depthReceiverRef;
    // depth::ReceiverRef colorReceiverRef;
    
    ofTexture depthTex;
    // ofTexture colorTex;
};

void ofApp::setup() {
  ofSetWindowShape(1280,480);
  // create tcp network receivers for both the depth and the color stream
  depthReceiverRef = depth::Receiver::createAndStart(remoteCamAddress, depthPort);
  // colorReceiverRef = depth::Receiver::createAndStart(remoteCamAddress, colorPort); // color stream isn't working yet on the transmitter side...
}

void ofApp::update() {
  // checks if our receivers have new data, if so these convenience methods
  // update (and allocate if necessary!) our textures.
  ofxDepthStream::loadDepthTexture(*depthReceiverRef, depthTex);
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

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
