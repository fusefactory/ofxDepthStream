// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
// local

class Recorder : public persee::Buffer  {

  public:

    Recorder(){
    }

    std::string getName() {
      int i=0;

      while(true){
        std::ifstream ifile("recording"+ofToString(i)+".txt");
        if(ifile){
          i++;
        } else {
          break;
        }
      }

      return "recording"+ofToString(i)+".txt";
    }

    void start() {
      std::string name = getName();
      outfile = new std::ofstream(name, std::ofstream::binary);

      frameCount = 0;
      byteCount = 0;
      startTime = ofGetElapsedTimeMillis();

      std::cout << "started recording to: " << name << std::endl;
    }

    void stop() {
      if(outfile){
        outfile->close();
        delete outfile;
        outfile=NULL;
        std::cout << "Recording stopped; recorded " << frameCount << " frames containing " << byteCount << " bytes of image data" << std::endl;
      }
    }

    bool record(const void* data, uint32_t size) {
      if(!outfile)
        return false;

      uint32_t t = ofGetElapsedTimeMillis() - startTime;
      outfile->write((const char*)&t, sizeof(t)); // 4 byte timestamp
      outfile->write((const char*)&size, sizeof(size)); // 4 byte frame size
      outfile->write((const char*)data, size); // frame body
      frameCount+=1;
      byteCount+=size;
      return true;
    }

    // persee::Buffer interface
    virtual void take(const void* data, size_t size) override {
      this->record(data, size);
      // call parent logic (forward data to any target set using our persee::Buffer::setOutputTo method)
      persee::Buffer::take(data, size);
    }

  private:
    uint64_t startTime;
    std::ofstream* outfile;
    size_t frameCount=0;
    size_t byteCount=0;
};

#include <chrono>

class Playback : public persee::Buffer {

  public:

    typedef std::function<void(void*, size_t)> FrameCallback;

  public:

    struct Frame {
      static const size_t BUF_SIZE=(1280*720*3);
      char buffer[BUF_SIZE];
      uint32_t size, time;
    };

    ~Playback(){
      stop(true);
    }

    std::string getName() {
      int i=0;

      while(true){
        std::ifstream ifile("recording"+ofToString(i)+".txt");
        if(ifile){
          i++;
        } else {
          break;
        }
      }

      return "recording"+ofToString(i-1)+".txt";
    }

    void start() {
      std::string name = getName();
      infile = new std::ifstream(name, std::ofstream::binary);

      frameCount=0;
      bPlaying=true;
      startTime = ofGetElapsedTimeMillis();

      std::cout << "started playback of: " << name << std::endl;
      this->update();
    }

    void startThreaded() {
      this->thread = new std::thread(std::bind(&Playback::threadFunc, this));
    }

    void stop(bool wait=true) {
      bPlaying=false;

      if(infile){
        infile->close();
        delete infile;
        infile=NULL;
      }

      if(wait){
        if(thread) {
          thread->join();
          delete thread;
          thread=NULL;
        }
      }
    }

    bool update(FrameCallback inlineCallback=nullptr) {
      if(!bPlaying) return false;

      if(!nextFrame){
        nextFrame=readFrame();

        if(!nextFrame){
          bPlaying=false;
          this->onEnd();
          return false;
        }
      }

      if(nextFrame) {
        auto t = ofGetElapsedTimeMillis() - startTime;

        if(t > nextFrame->time) {
          if(inlineCallback)
            inlineCallback(nextFrame->buffer, nextFrame->size);

          // implement/execute our persee::Buffer interface
          this->take(nextFrame->buffer, nextFrame->size);

          nextFrame = NULL;
          frameCount += 1;
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

    void threadFunc() {
      this->start();
      while(this->bPlaying){
        this->update();
        std::this_thread::sleep_for(std::chrono::duration<long, std::milli>(50l));
      }
    }

    void onEnd() {
      stop(false);
      if(bLoop) {
        if(frameCount == 0) {
          std::cout << "[persee::Playback] file appears empty" << std::endl;
        } else {
          start();
        }
      }
    }

  private:
    bool bPlaying=false;
    bool bLoop=true;
    uint64_t startTime;
    std::ifstream* infile;
    // size_t frameCount=0;
    Frame frame;
    size_t frameCount;
    Frame* nextFrame=NULL;

    std::thread* thread=NULL;
};

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
    std::string address = "192.168.1.226";
    // ofxOrbbecPersee::DepthStreamRef depthStreamRef;

    persee::ReceiverRef receiverRef;
    persee::Buffer depthBuffer;

    ofTexture depthTex;
    ofPixels depthPixels;

    bool bRecording=false;
    Recorder recorder;

    bool bPlaying=false;
    Playback playback;
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
    this->depthBuffer.take(data, size);
  });

  // check if our depth buffer has a frame (either through our network receiver and recorder, or throuh our playback);
  // persee::emptybuffer will execute the given lambda only if there is data in the buffer, and also empty the buffer
  persee::emptyBuffer(depthBuffer, [this](const void* data, size_t size){
    // returns shared_ptr<persee::Frame> with inflated data
    persee::inflate(data, size)
    // returns shared_ptr<persee::Frame> with 1-byte grayscale data
    ->convert(persee::grayscale255bitConverter(this->depthPixels.getWidth(), this->depthPixels.getHeight()))
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
