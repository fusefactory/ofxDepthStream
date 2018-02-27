// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

    // void keyPressed(int key);
    // void keyReleased(int key);
    // void mouseMoved(int x, int y );
    // void mouseDragged(int x, int y, int button);
    // void mousePressed(int x, int y, int button);
    // void mouseReleased(int x, int y, int button);
    // void mouseEntered(int x, int y);
    // void mouseExited(int x, int y);
    // void windowResized(int w, int h);
    // void dragEvent(ofDragInfo dragInfo);
    // void gotMessage(ofMessage msg);

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
  colorReceiverRef = persee::Receiver::createAndStart(perseeAddress, colorPort);
}

void ofApp::update() {
  // checks if our receivers have new data, if so these convenience methods
  // update (and allocate if necessary!) our textures.
  ofxOrbbecPersee::loadGrayscaleTexture(*depthReceiverRef, depthTex);
  ofxOrbbecPersee::loadColorTexture(*colorReceiverRef, colorTex);
}

void ofApp::draw() {
  ofBackground(0);

  if(depthTex.isAllocated()) {
    depthTex.draw(0, 0);
  }

  if(colorTex.isAllocated()) {
    colorTex.draw(0, 0);
  }
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
