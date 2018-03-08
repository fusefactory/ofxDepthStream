// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

  private: // attributes
    std::string perseeAddress = "persee.local"; //"192.168.1.226"; // "127.0.0.1";
    int depthPort = 4445;
    int colorPort = 4446;

    persee::ReceiverRef depthReceiverRef, colorReceiverRef;
    ofTexture depthTex, colorTex;
};

void ofApp::setup() {
  ofSetWindowShape(1280,480);
  // create tcp network receivers for both the depth and the color stream
  depthReceiverRef = persee::Receiver::createAndStart(perseeAddress, depthPort);
  colorReceiverRef = persee::Receiver::createAndStart(perseeAddress, colorPort); // color stream isn't working yet on the transmitter side...
}

void ofApp::update() {
  // checks if our receivers have new data, if so these convenience methods
  // update (and allocate if necessary!) our textures.
  ofxOrbbecPersee::loadDepthTexture(*depthReceiverRef, depthTex);
  ofxOrbbecPersee::loadColorTexture(*colorReceiverRef, colorTex);
}

void ofApp::draw() {
  ofBackground(0);

  if(depthTex.isAllocated()) {
    depthTex.draw(0, 0);
  }

  if(colorTex.isAllocated()) {
    colorTex.draw(640, 0);
  }
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
