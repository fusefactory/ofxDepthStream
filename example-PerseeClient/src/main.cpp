// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
// local

class ofApp : public ofBaseApp{

  public: // methods
    ofApp(int argc, char** argv);
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
    std::string perseeHost = "127.0.0.1"; //"192.168.1.226";
    int perseePort = 4444; // default
    ofxOrbbecPersee::Client client;
    ofxOrbbecPersee::DepthStreamRef depthStreamRef;
    ofxOrbbecPersee::ColorStreamRef colorStreamRef;
};

ofApp::ofApp(int argc, char** argv) {
  if(argc > 1) perseeHost = argv[1];
  if(argc > 2) perseePort = std::atoi(argv[2]);
}

void ofApp::setup() {
  ofSetWindowShape(1280,480);
  // use all default options (port 4444, only depth stream enabled, 30fps), only specify the Persee's IP
  client.setup(perseeHost, perseePort);
  depthStreamRef = client.createDepthStream(); // 640x480 by default
  colorStreamRef = client.createColorStream(); // 1280x720 by default
}

void ofApp::update() {
  depthStreamRef->update();
  colorStreamRef->update();
}

void ofApp::draw() {
  ofBackground(0);

  auto tex = depthStreamRef->getTexture();

  if(tex.isAllocated()) {
    tex.draw(0, 0);
  }

  auto tex2 = colorStreamRef->getTexture();

  if(tex2.isAllocated()) {
    tex2.draw(650, 0, tex2.getWidth(), tex2.getHeight());
  }
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp(argc, argv));
}
