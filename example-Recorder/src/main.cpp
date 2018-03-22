// OF
#include "ofMain.h"
// addons
#include "ofxDepthStream/ofxDepthStream.h"

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    // void exit(){} override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
    std::string getNewRecordingFilename();
    std::string getLastRecordingFilename();
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
    std::string address = "persee.local"; //"192.168.1.226";
    // ofxDepthStream::DepthStreamRef depthStreamRef;

    depth::ReceiverRef receiverRef;
    depth::Buffer depthBuffer;

    ofTexture depthTex;
    ofPixels depthPixels;

    bool bRecording=false;
    depth::Recorder recorder;

    bool bPlaying=false;
    depth::Playback playback;
};

void ofApp::setup() {
  ofSetWindowShape(640,480);

  // setup tcp network stream receiver
  receiverRef = depth::Receiver::createAndStart(address); // default port: 4445
  // pipe our tcp receiver into our recorder (will only record when its start method is called)
  receiverRef->setOutputTo(&recorder);
  // pipe our recorder into our depth buffer
  recorder.setOutputTo(&depthBuffer);
  // playback.setOutputTo(&depthBuffer);

  // create our texture
  depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
  depthTex.allocate(depthPixels);
}

void ofApp::update() {
  // update with inline frame callback
  playback.update([this](void* data, size_t size){
    // ofLogNotice() << "playback update";
    this->depthBuffer.write(data, size);
  });

  ofxDepthStream::loadDepthTexture(depthBuffer, depthTex);
}

void ofApp::draw() {
  ofBackground(0);

  if(depthTex.isAllocated()) {
    ofSetColor(255,255,255);
    depthTex.draw(0, 0);
  }

  if(bRecording) {
    ofSetColor(255,0,0,100);
    ofDrawRectangle(0,0,20,20);
  }

  if(bPlaying) {
    ofSetColor(0,255,0,100);
    ofDrawRectangle(0,0,20,20);
  }
}

void ofApp::keyPressed(int key) {
  // toggle record on/off
  if(key == 'r') {
    bRecording = !bRecording;
    if(bRecording) {
      recorder.start(ofToDataPath(getNewRecordingFilename()));
    } else {
      recorder.stop();
    }
  }

  // toggle playback on/off
  if(key=='p'){
    bPlaying = !bPlaying;
    if(bPlaying){
      // playback.startThreaded();
      playback.start(ofToDataPath(getLastRecordingFilename()));
      // "disconnected" our recorder from our depth buffer
      recorder.setOutputTo(NULL);
    } else {
      playback.stop();
      // "reconnect" our recorder to our depth buffer
      recorder.setOutputTo(&depthBuffer);
    }
  }
}

std::string ofApp::getNewRecordingFilename() {
  int i=0;

  while(true){
    if(ofFile::doesFileExist("recording"+ofToString(i)+".txt")){
      i++;
    } else {
      break;
    }
  }

  return "recording"+ofToString(i)+".txt";
}

std::string ofApp::getLastRecordingFilename() {
  int i=0;

  while(true){
    if(ofFile::doesFileExist("recording"+ofToString(i)+".txt")){
      i++;
    } else {
      break;
    }
  }

  return "recording"+ofToString(i-1)+".txt";
}


//========================================================================

int main(){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
