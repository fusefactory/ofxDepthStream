// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"

class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    // void exit(){} override;
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
    std::string address = "persee.local"; //"192.168.1.226";
    // ofxOrbbecPersee::DepthStreamRef depthStreamRef;

    persee::ReceiverRef receiverRef;
    persee::Buffer depthBuffer;

    ofTexture depthTex;
    ofPixels depthPixels;

    bool bRecording=false;
    persee::Recorder recorder;

    bool bPlaying=false;
    persee::Playback playback;
};

void ofApp::setup() {
  ofSetWindowShape(640,480);

  // setup tcp network stream receiver
  receiverRef = persee::Receiver::createAndStart(address); // default port: 4445
  // pipe our tcp receiver into our recorder (will only record when its start method is called)
  receiverRef->setOutputTo(&recorder);
  // pipe our recorder into our depth buffer
  recorder.setOutputTo(&depthBuffer);

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

  // check if our depth buffer has a frame (either through our network receiver and recorder, or throuh our playback);
  // persee::emptybuffer will execute the given lambda only if there is data in the buffer, and also empty the buffer
  persee::emptyBuffer(depthBuffer, [this](const void* data, size_t size){
    // returns shared_ptr<persee::Frame> with inflated data
    persee::inflate(data, size)
    // returns shared_ptr<persee::Frame> with 1-byte grayscale data
    ->convert(persee::grayscale8bitConverter(this->depthPixels.getWidth(), this->depthPixels.getHeight()))
    // load grayscale data into our ofTexture instance
    ->convert<void>([this](const void* data, size_t size){
      // ofLogNotice() << "buffer to tex onversion update: " << size;
      this->depthPixels.setFromPixels((const unsigned char *)data, depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
      this->depthTex.loadData(depthPixels);
    });
  });
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
      recorder.start();
    } else {
      recorder.stop();
    }
  }

  // toggle playback on/off
  if(key=='p'){
    bPlaying = !bPlaying;
    if(bPlaying){
      // playback.startThreaded();
      playback.start();
      // "disconnected" our recorder from our depth buffer
      recorder.setOutputTo(NULL);
    } else {
      playback.stop();
      // "reconnect" our recorder to our depth buffer
      recorder.setOutputTo(&depthBuffer);
    }
  }
}

//========================================================================

int main(){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
