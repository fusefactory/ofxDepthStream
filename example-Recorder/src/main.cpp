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
      int i=0;

      while(true){
        std::ifstream ifile("recording"+ofToString(i)+".txt");
        if(ifile){
          i++;
        } else {
          break;
        }
      }

      outfile = new std::ofstream("recording"+ofToString(i)+".txt",std::ofstream::binary);

      startTime = ofGetElapsedTimeMillis();
      std::cout << "started recording" << std::endl;
    }

    void stop() {
      if(outfile){
        outfile->close();
        delete outfile;
        outfile=NULL;
        std::cout << "stopped recording" << std::endl;
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

class Playback {

  public:

    typedef std::function<void(void*, size_t)> FrameCallback;

  public:

    struct Frame {
      static const size_t BUF_SIZE=(1280*720*3);
      char buffer[BUF_SIZE];
      uint32_t size, time;
    };

    void start() {
      infile = new std::ifstream("recording.txt", std::ofstream::binary);
      startTime = ofGetElapsedTimeMillis();
      std:cout << "started playback" << std::endl;
      bPlaying=true;
      this->update();
    }

    void stop() {
      bPlaying=false;
    }

    bool update(FrameCallback inlineCallback=nullptr) {
      if(!nextFrame){
        nextFrame=readFrame();

        if(!nextFrame){
          bPlaying=false;
          return false;
        }
      }

      if(nextFrame) {
        auto t = ofGetElapsedTimeMillis() - startTime;

        if(t > nextFrame->time) {
          if(inlineCallback)
            inlineCallback(nextFrame->buffer, nextFrame->size);

          if(frameCallback)
            frameCallback(nextFrame->buffer, nextFrame->size);

          nextFrame = NULL;
          return true;
        }
      }

      return false;
    }

  protected:
    Frame* readFrame() {
      if(infile->read((char*)&frame.time, sizeof(uint32_t)) &&
        infile->read((char*)&frame.size, sizeof(uint32_t)) &&
        infile->read((char*)&frame.buffer, frame.size)) {
        return &frame;
      }

      return NULL;
    }

  private:
    bool bPlaying=false;
    uint64_t startTime;
    std::ifstream* infile;
    size_t frameCount=0;
    Frame frame;
    Frame* nextFrame=NULL;
    FrameCallback frameCallback;
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
    ofxOrbbecPersee::DepthStreamRef depthStreamRef;

    bool bRecording=false;
    Recorder recorder;

    bool bPlaying=false;
    Playback playback;
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
  if(bPlaying){
    playback.update([this](void* data, size_t size){
      std::static_pointer_cast<ofxOrbbecPersee::StreamReceiver>(depthStreamRef->getAddons()[0])->process(data, size);
    });
  }

  depthStreamRef->update();
}

void ofApp::draw() {
  ofBackground(0);

  auto tex = depthStreamRef->getTexture();

  if(tex.isAllocated()) {
    ofSetColor(255,255,255);
    tex.draw(0, 0);
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
  if(key == 'r') {
    bRecording = !bRecording;
    if(bRecording) {
      recorder.start();
    } else {
      recorder.stop();
    }
  }

  if(key=='p'){
    bPlaying = !bPlaying;
    if(bPlaying){
      playback.start();
      std::static_pointer_cast<ofxOrbbecPersee::StreamReceiver>(depthStreamRef->getAddons()[0])->getReceiver().stop();
    } else {
      playback.stop();
      std::static_pointer_cast<ofxOrbbecPersee::StreamReceiver>(depthStreamRef->getAddons()[0])->getReceiver().start();
    }
  }
}

//========================================================================

int main(){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp());
}
