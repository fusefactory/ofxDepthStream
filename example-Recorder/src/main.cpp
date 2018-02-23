// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
// local

class Recorder {

  public:

    Recorder(){
    }

    void start() {
      outfile = new std::ofstream("recording.txt",std::ofstream::binary);
      startTime = ofGetElapsedTimeMillis();
      std:cout << "started recording" << std::endl;
    }

    void stop() {
      if(outfile){
        outfile->close();
        delete outfile;
        outfile=NULL;
        std:cout << "stopped recording" << std::endl;
      }
    }

    bool record(const void* data, uint32_t size) {
      uint32_t t = ofGetElapsedTimeMillis() - startTime;

      if(outfile) {
        outfile->write((const char*)&t, sizeof(t)); // 4 byte timestamp
        outfile->write((const char*)&size, sizeof(size)); // 4 byte frame size
        outfile->write((const char*)data, size); // frame body
        frameCount+=1;
        return true;
      }

      return false;
    }

  private:
    uint64_t startTime;
    std::ofstream* outfile;
    size_t frameCount=0;
};

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
    std::string address = "192.168.1.226";
    bool bRecording=false;
    ofxOrbbecPersee::DepthStreamRef depthStreamRef;

    Recorder recorder;
};

void ofApp::setup() {
  ofSetWindowShape(640,480);
  // use all default options (port 4444, only depth stream enabled, 30fps), only specify the Persee's IP

  ofxOrbbecPersee::Client client;
  client.setup(address);
  depthStreamRef = client.createDepthStream(); // 640x480 by default

  std::static_pointer_cast<ofxOrbbecPersee::StreamReceiver>(depthStreamRef->getAddons()[0])->getReceiver().setFrameCallback([this](const void* data, size_t size){
    if(recorder.record(data, size)) {
      ofLogNotice() << "recorded " << size << "-byte frame";
    }
  });
}

void ofApp::update() {
  depthStreamRef->update();
}

void ofApp::draw() {
  ofBackground(0);

  auto tex = depthStreamRef->getTexture();

  if(tex.isAllocated()) {
    tex.draw(0, 0);
  }
}

void ofApp::keyPressed(int key) {
  if(key == 'r') {
    bRecording = !bRecording;
    if(bRecording) {
      recorder.start();
    } else {
      recorder.stop();
    }
  }
}

//========================================================================

int main(){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
