// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
// local

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
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
    persee::ReceiverRef depthReceiverRef;

    // ofEasyCam cam;
    ofTexture depthTex, edgeTex;

    bool bDrawDepth=true, bDrawEdge=true;
};

void ofApp::setup() {
  ofSetWindowTitle("ofxOrbbecPersee - Mesh Example");
  ofSetWindowShape(1290,720);
  ofSetVerticalSync(true);

  // create tcp network receivers for both the depth and the color stream
  depthReceiverRef = persee::Receiver::createAndStart(perseeAddress, depthPort);
}

void ofApp::update() {
  // emptyAndInflateBuffer only executes the given lambda when a frame was received and could successfully be inflated
  persee::emptyAndInflateBuffer(*depthReceiverRef, [this](const void* data, size_t size){
    if (bDrawDepth) {
      ofxOrbbecPersee::loadGrayscaleTexture(depthTex, data, size);
    }

    if (bDrawEdge) {
      ofxOrbbecPersee::loadDepthTexture(edgeTex, data, size);
    }
  });
}

void ofApp::draw() {
  ofBackground(0);

  // cam.begin();
    if (bDrawDepth) {
      if(depthTex.isAllocated()) {
        // ofPushMatrix();
        // ofScale(-1.0f, -1.0f, 1.0f);
        depthTex.draw(650, 0);
        // ofPopMatrix();
      }
    }

    ofDrawBitmapString("ofTexture: OF_IMAGE_GRAYSCALE", 660, 500);

    if (bDrawEdge) {
      if(edgeTex.isAllocated()) {
        // ofPushMatrix();
        // ofScale(-1.0f, -1.0f, 1.0f);
        edgeTex.draw(0,0);
        // ofPopMatrix();
      }
    }

    ofDrawBitmapString("ofTexture: GL_RGB", 10, 500);
  // cam.end();

  ofDrawBitmapString("Press d or D to toggle draw modes", 10, 680);
  ofDrawBitmapString("Press ,/< or ./> to de-/increase depthFactor", 10, 700);
}

void ofApp::keyPressed(int key) {
  if (key == 'd') { bDrawEdge = !bDrawEdge; }
  if (key == 'D') { bDrawDepth = !bDrawDepth; }
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
