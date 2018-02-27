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
    std::string perseeAddress = "192.168.1.226"; // "127.0.0.1";
    int depthPort = 4445;

    ofVboMesh mesh;
    float depthFactor=0.08f;
    ofEasyCam cam;
    ofTexture depthTex;

    persee::ReceiverRef depthReceiverRef;

    bool bDrawPoints=true, bDrawDepth=true;
};

void ofApp::setup() {
  ofSetWindowTitle("ofxOrbbecPersee - Mesh Example");
  ofSetWindowShape(1280,720);
  ofSetVerticalSync(true);

  // create tcp network receivers for both the depth and the color stream
  depthReceiverRef = persee::Receiver::createAndStart(perseeAddress, depthPort);
  mesh.setMode(OF_PRIMITIVE_POINTS);
}

void ofApp::update() {
  // emptyAndInflateBuffer only executes the given lambda when a frame was received and could successfully be inflated
  persee::emptyAndInflateBuffer(*depthReceiverRef, [this](const void* data, size_t size){

    if (bDrawDepth) {
      ofxOrbbecPersee::loadGrayscaleTexture(depthTex, data, size);
    }

    if (bDrawPoints) {
      const uint16_t* pointData = (const uint16_t*)data;
      int maxDepth = 1500;
      size_t w = 640;
      size_t h = 480;
      // size_t itemSize = size / (w*h); // 2

      this->mesh.clear();

      for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
          // ofVec3f p = astra.getWorldCoordinateAt(x, y);
          uint16_t val = pointData[y*w+x] * this->depthFactor;
          ofVec3f p(x,y,val);

          // if (p.z == 0) continue;
          // if (p.z > maxDepth) continue;

          this->mesh.addVertex(p);

          // if (bPointCloudUseColor) {
          //   mesh.addColor(astra.getColorImage().getColor(x, y));
          // } else {
          //   float hue  = ofMap(p.z, 0, maxDepth, 0, 255);
          //   mesh.addColor(ofColor::fromHsb(hue, 255, 255));
          // }
          float hue  = ofMap(p.z, 0, maxDepth, 0, 255);
          this->mesh.addColor(ofColor::fromHsb(hue, 255, 255));
        }
      }
    }
  });
}

void ofApp::draw() {
  ofBackground(0);

  cam.begin();
    if (bDrawDepth) {
      if(depthTex.isAllocated()) {
        depthTex.draw(0, 0);
      }
    }

    if (bDrawPoints) {
      ofEnableDepthTest();
      ofRotateY(180);
      ofScale(1.5, 1.5);

      mesh.draw();

      // for (auto& hand : astra.getHandsWorld()) {
      //   auto& pos = hand.second;
      //   ofSetColor(ofColor::white);
      //   ofDrawCircle(pos, 10);
      //
      //   stringstream ss;
      //   ss << "id: " << hand.first << endl;
      //   ss << "pos: " << hand.second;
      //   ofDrawBitmapString(ss.str(), pos.x, pos.y - 30, pos.z);
      // }

      ofDisableDepthTest();
    }
  cam.end();

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
