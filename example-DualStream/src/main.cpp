// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
#include "ofxHistoryPlot.h"
#include "ofxGui.h"
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

    NetworkSource src1 = {"persee.local", 4445}; // orbbec
    NetworkSource src2 = {"192.168.1.77", 4444}; // kinect

    ofTexture textures[2];
    persee::ReceiverRef receiverRefs[2];
    persee::Recorder recorders[2];
    persee::Playback playbacks[2];

    // plot/speed
    float avgBytesPerFrame[2] = { 0.0f, 0.0f };
    float avgBytesPerSecond[2] { 0.0f, 0.0f };
    unsigned int bytesThisSecond[2] { 0,0 };
    float secondTimer=0;

    ofxHistoryPlot* plots[4];

    // gui
    typedef struct {
      ofParameterGroup params;
      ofParameter<int> minDistance, maxDistance, vertCorrection;
      ofParameter<float> keystone, margins[4];
      ofxPanel gui;
    } ParsStruct;

    // ofParameterGroup paramsGui;
    ParsStruct pars[2];
    // ofxPanel gui;
    bool bDrawGui=true, bDrawHelp=false;
};

void ofApp::setup() {
  ofSetWindowTitle("ofxOrbbecPersee - DualStream Example");
  ofSetWindowShape(1290,750);
  ofSetVerticalSync(true);

  receiverRefs[0] = persee::Receiver::createAndStart(src1.address, src1.port);
  receiverRefs[0]->setOutputTo(&recorders[0]);
  playbacks[0].setOutputTo(&recorders[0]);

  receiverRefs[1] = persee::Receiver::createAndStart(src2.address, src2.port);
  receiverRefs[1]->setOutputTo(&recorders[1]);
  playbacks[1].setOutputTo(&recorders[1]);

  // gui
  plots[0] = new ofxHistoryPlot( NULL, "kB/frame", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[0]->setColor( ofColor(0,255,255) );
  plots[0]->setRange(0, 400);
  plots[1] = new ofxHistoryPlot( NULL, "kB/frame", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[1]->setColor( ofColor(255,0,255) );
  plots[1]->setRange(0, 400);
  plots[2] = new ofxHistoryPlot( NULL, "kB/sec", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[2]->setColor( ofColor(0,255,255) );
  plots[2]->setRange(0, 7000);
  plots[3] = new ofxHistoryPlot( NULL, "kB/sec", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
  plots[3]->setColor( ofColor(255,0,255) );
  plots[3]->setRange(0, 7000);

  auto cfg = [](ParsStruct& pars) {
    pars.params.add(pars.minDistance.set("minDistance", 50, 0, 15000));
    pars.params.add(pars.maxDistance.set("maxDistance", 8000, 0, 15000));
    pars.params.add(pars.vertCorrection.set("vertCorrection", 1, 0, 10));
    pars.params.add(pars.keystone.set("keystone", 0.0f, 0.0f, 10.0f));
    pars.params.add(pars.margins[0].set("marginT", 0.0f, 0.0f, 400.0f));
    pars.params.add(pars.margins[1].set("marginR", 0.0f, 0.0f, 400.0f));
    pars.params.add(pars.margins[2].set("marginB", 0.0f, 0.0f, 400.0f));
    pars.params.add(pars.margins[3].set("marginL", 0.0f, 0.0f, 400.0f));
    pars.gui.setup(pars.params);
  };

  cfg(pars[0]);
  cfg(pars[1]);
  pars[1].gui.setPosition(650,10);

  // paramsGui.setName("App");
  // paramsGui.add(pars[0].params);
  // paramsGui.add(pars[1].params);
  // gui.setup(paramsGui);
}

void ofApp::update() {
  for(int i=0; i<2; i++){
    auto frameRef = recorders[i].getRef();

    persee::emptyAndInflateBuffer(recorders[i], [this, frameRef, i](const void* data, size_t size){
      ofxOrbbecPersee::loadDepthTexture(
        textures[i],
        data, size/*,
        ofxOrbbecPersee::DepthLoaderOpts()
          .setMinDistance(this->pars[i].minDistance)
          .setMaxDistance(this->pars[i].maxDistance)
          .setVertCorrection(this->pars[i].vertCorrection)
          .setKeystone(this->pars[i].keystone)
          .setMarginTop(this->pars[i].margins[0])
          .setMarginRight(this->pars[i].margins[1])
          .setMarginBottom(this->pars[i].margins[2])
          .setMarginLeft(this->pars[i].margins[3])*/);
      plots[i]->update(frameRef->size()/1000.0f); // write original (compressed) size to our plot
      this->avgBytesPerFrame[i] = (this->avgBytesPerFrame[i] * 9.0f + frameRef->size()) / 10.0f;
      this->bytesThisSecond[i] += frameRef->size();
    });
  }

  auto t = ofGetElapsedTimeMillis();
  if(t - secondTimer > 1000.0f) {
    this->avgBytesPerSecond[0] = ((this->avgBytesPerSecond[0] * 4) + this->bytesThisSecond[0]) / 5.0f;
    this->avgBytesPerSecond[1] = ((this->avgBytesPerSecond[1] * 4) + this->bytesThisSecond[1]) / 5.0f;
    secondTimer = t;
    this->bytesThisSecond[0] = 0;
    this->bytesThisSecond[1] = 0;
    plots[2]->update(this->avgBytesPerSecond[0]/1000.0f);
    plots[3]->update(this->avgBytesPerSecond[1]/1000.0f);
  }
}

void ofApp::draw() {
  ofBackground(0);

  { // left
    if(textures[0].isAllocated()) {
      textures[0].draw(0, 0);
    }

    stringstream ss;
    ss << ofToString(avgBytesPerFrame[0]/1000.0f) + " kB/f - ";
    ss << ofToString(avgBytesPerSecond[0]/1000.0f) + " kB/s - ";
    ss << playbacks[0].isPlaying() ? ofFile(playbacks[0].getFilename()).getBaseName() : src1.address+":"+ofToString(src1.port);
    ofDrawBitmapString(ss.str(), 10, 500);
    plots[0]->draw(10, 540, 400, 100);
    plots[2]->draw(10, 640, 400, 100);
  }

  { // right
    if(textures[1].isAllocated()) {
      textures[1].draw(650, 0);
    }

    stringstream ss;
    ss << ofToString(avgBytesPerFrame[1]/1000.0f) + " kB/f - ";
    ss << ofToString(avgBytesPerSecond[1]/1000.0f) + " kB/s - ";
    ss << playbacks[1].isPlaying() ? ofFile(playbacks[1].getFilename()).getBaseName() : src2.address+":"+ofToString(src2.port);
    ofDrawBitmapString(ss.str(), 650, 520);
    plots[1]->draw(650, 540, 400, 100);
    plots[3]->draw(650, 640, 400, 100);
  }

  if(bDrawGui) {
    pars[0].gui.draw();
    pars[1].gui.draw();
  }

  if(bDrawHelp){
    stringstream ss;
    ss << "h - toggle this help" << std::endl;
    ss << "g - toggle GUI" << std::endl;
    ss << "m - mute all inputs (both recordings and network streams)" << std::endl;
    ss << "M - unmute (resume network streams)" << std::endl;
    ofDrawBitmapString(ss.str(), 10, ofGetWindowHeight() - 60);
  }
}

void ofApp::keyPressed(int key) {
  // 'mute' stop both playbacks and network streams
  if(key == 'm') {
    stopPlayback(0);
    stopPlayback(1);
    receiverRefs[0]->setOutputTo(NULL);
    receiverRefs[1]->setOutputTo(NULL);
  }

  // 'unmute' stop both playbacks, start network streams
  if(key == 'M') {
    stopPlayback(0);
    stopPlayback(1);
  }

  // 'stop' stop playbacks (resume network streams)
  if(key == 's') {
    stopPlayback(0);
    stopPlayback(1);
  }

  // 'gui' toggle gui on/off
  if(key == 'g') bDrawGui = !bDrawGui;
  // 'help' toggle draw help on/off
  if(key == 'h') bDrawHelp = !bDrawHelp;
}

// Start recorded playback ('which' can be 0 for left, 1 for right). Stops network streaming.
void ofApp::startPlayback(int which, const std::string& filename) {
  playbacks[which].startThreaded(filename);
  // stop network stream (through recorder)
  receiverRefs[which]->setOutputTo(NULL);
}

// Stop recorded playback ('which' can be 0 for left, 1 for right). Resumes network streaming.
void ofApp::stopPlayback(int which) {
  playbacks[which].stop();
  // reconnect network stream (through recorder)
  receiverRefs[which]->setOutputTo(&recorders[which]);
}

// Drag recording data file; start recording playback
void ofApp::dragEvent(ofDragInfo dragInfo) {
  if(dragInfo.files.size() < 1) return;
  this->startPlayback(dragInfo.position.x < (ofGetWindowWidth() >> 1) ? 0 : 1, dragInfo.files[0]);
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
