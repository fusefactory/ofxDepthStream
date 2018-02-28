// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
#include "ofxHistoryPlot.h"
// local

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
    void startPlayback(int which, const std::string& filename);
    void stopPlayback(int which);
    // void keyReleased(int key);
    // void mouseMoved(int x, int y );
    // void mouseDragged(int x, int y, int button);
    // void mousePressed(int x, int y, int button);
    // void mouseReleased(int x, int y, int button);
    // void mouseEntered(int x, int y);
    // void mouseExited(int x, int y);
    // void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo) override;
    // void gotMessage(ofMessage msg);

  private: // attributes

    typedef struct {
      std::string address;
      int port;
    } NetworkSource;

    NetworkSource src1 = {"persee.local", 4445};
    NetworkSource src2 = {"192.168.1.77", 4444};

    persee::ReceiverRef receiverRefs[2];
    persee::Recorder recorders[2];
    persee::Playback playbacks[2];
    float avgBytesPerFrame[2] = { 0.0f, 0.0f };

    ofxHistoryPlot* plots[2];

    // ofEasyCam cam;
    ofTexture textures[2];
};

void ofApp::setup() {
  ofSetWindowTitle("ofxOrbbecPersee - Mesh Example");
  ofSetWindowShape(1290,720);
  ofSetVerticalSync(true);

  receiverRefs[0] = persee::Receiver::createAndStart(src1.address, src1.port);
  receiverRefs[0]->setOutputTo(&recorders[0]);
  playbacks[0].setOutputTo(&recorders[0]);

  receiverRefs[1] = persee::Receiver::createAndStart(src2.address, src2.port);
  receiverRefs[1]->setOutputTo(&recorders[1]);
  playbacks[1].setOutputTo(&recorders[1]);

  plots[0] = new ofxHistoryPlot( NULL, "bytes/frame", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[1] = new ofxHistoryPlot( NULL, "bytes/frame", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[1]->setColor( ofColor(0,0,255) );
}

void ofApp::update() {
  for(int i=0; i<2; i++){
    auto frameRef = recorders[i].getRef();

    persee::emptyAndInflateBuffer(recorders[i], [this, frameRef, i](const void* data, size_t size){
      ofxOrbbecPersee::loadDepthTexture(textures[i], data, size);
      plots[i]->update(frameRef->size()); // write original (compressed) size to our plot
      this->avgBytesPerFrame[i] = (this->avgBytesPerFrame[i] * 9.0f + frameRef->size()) / 10.0f;
    });
  }
}

void ofApp::draw() {
  ofBackground(0);

  if(textures[0].isAllocated()) {
    textures[0].draw(0, 0);
  }

  std::string speed = ofToString(avgBytesPerFrame[0]) + "bpf - ";
  std::string source = playbacks[0].isPlaying() ? ofFile(playbacks[0].getFilename()).getBaseName() : src1.address+":"+ofToString(src1.port);
  ofDrawBitmapString(speed + source, 10, 500);
  plots[0]->draw(10, 540, 400, 150);

  if(textures[1].isAllocated()) {
    textures[1].draw(650, 0);
  }

  speed = ofToString(avgBytesPerFrame[1]) + "bpf - ";
  source = playbacks[1].isPlaying() ? ofFile(playbacks[1].getFilename()).getBaseName() : src2.address+":"+ofToString(src2.port);
  ofDrawBitmapString(speed + source, 650, 520);
  plots[1]->draw(650, 540, 400, 150);
}

void ofApp::keyPressed(int key) {
  // if (key == 'd') { bDrawEdge = !bDrawEdge; }
  // if (key == 'D') { bDrawDepth = !bDrawDepth; }
  if(key == 'm') {
    stopPlayback(0);
    stopPlayback(1);
    receiverRefs[0]->setOutputTo(NULL);
    receiverRefs[1]->setOutputTo(NULL);
  }

  if(key == 'M') {
    stopPlayback(0);
    stopPlayback(1);
  }

  if(key == 's') {
    stopPlayback(0);
    stopPlayback(1);
  }
}

void ofApp::startPlayback(int which, const std::string& filename) {
  playbacks[which].startThreaded(filename);
  // stop network stream (through recorder)
  receiverRefs[which]->setOutputTo(NULL);
}

void ofApp::stopPlayback(int which) {
  playbacks[which].stop();
  // reconnect network stream (through recorder)
  receiverRefs[which]->setOutputTo(&recorders[which]);
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
  if(dragInfo.files.size() < 1) return;
  this->startPlayback(dragInfo.position.x < (ofGetWindowWidth() >> 1) ? 0 : 1, dragInfo.files[0]);
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
