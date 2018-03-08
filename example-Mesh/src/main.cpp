// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"

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

    // persee/depth data
    std::string perseeAddress = "persee.local"; // "192.168.1.226"; // "127.0.0.1";
    int depthPort = 4445;
    persee::ReceiverRef receiverRef;

    // data-containers; mesh/texture
    ofVboMesh mesh;
    float depthFactor=-0.08f;
    ofTexture depthTex;

    // rendering
    ofEasyCam cam;
    bool bDrawPoints=true, bDrawDepth=true;
};

void ofApp::setup() {
  ofSetWindowTitle("ofxOrbbecPersee - Mesh Example");
  ofSetWindowShape(1280,720);
  ofSetVerticalSync(true);

  mesh.setMode(OF_PRIMITIVE_POINTS);

  // create tcp network receiver for the depth image stream
  receiverRef = persee::Receiver::createAndStart(perseeAddress, depthPort);
}

void ofApp::update() {
  // emptyAndInflateBuffer only executes the given lambda when a frame was received
  // and if that frame could successfully be inflated (data is compressed to conserve network bandwidth)
  persee::emptyAndInflateBuffer(*receiverRef, [this](const void* data, size_t size){
    // load the received data into a grayscale texture
    if (bDrawDepth) {
      ofxOrbbecPersee::loadDepthTexture(depthTex, data, size);
    }

    // load the received data into our point-cloud mesh
    if (bDrawPoints) {
      ofxOrbbecPersee::loadMesh(mesh, data, size, ofxOrbbecPersee::MeshLoaderOpts()
        .setDepthFactor(depthFactor));
    }
  });
}

void ofApp::draw() {
  ofBackground(0);

  {
    cam.begin();

    if (bDrawDepth && depthTex.isAllocated()) {
      ofPushMatrix();
      ofScale(-1.0f, -1.0f, 1.0f);
      depthTex.draw(0, 0);
      ofPopMatrix();
    }

    if (bDrawPoints) {
      ofEnableDepthTest();
      ofRotateY(180);
      ofScale(1.5, 1.5);
      mesh.draw();
      ofDisableDepthTest();
    }

    cam.end();
  }

  ofDrawBitmapString("Press d or D to toggle draw modes", 10, 680);
  ofDrawBitmapString("Press ,/< or ./> to de-/increase depthFactor", 10, 700);
}

void ofApp::keyPressed(int key) {
  if (key == 'd') { bDrawPoints = !bDrawPoints; }
  if (key == 'D') { bDrawDepth = !bDrawDepth; }

  if (key == ',' || key == '<') { depthFactor -= 0.01f; ofLogNotice() << "depthFactor: " << depthFactor; }
  if (key == '.' || key == '>') { depthFactor += 0.01f; ofLogNotice() << "depthFactor: " << depthFactor; }
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
