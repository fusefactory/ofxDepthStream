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
    std::string perseeHost = "192.168.1.172";
    ofxOrbbecPersee::Client client;
};

void ofApp::setup() {
  // use all default options (port 4444, only depth stream enabled, 30fps), only specify the Persee's IP
  client.setup(perseeHost);
}

void ofApp::update() {
}

void ofApp::draw() {
  ofBackground(0);
  client.getDepthStream().draw();
}

//========================================================================

int main( ){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
