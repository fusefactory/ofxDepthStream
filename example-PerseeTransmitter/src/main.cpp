// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
#include "ofxTCPServer.h"
// local
#include "persee/protocol.h"
#include "persee/CamInterface.h"
#include "persee/Formatter.h"
#include "persee/Compressor.h"

using namespace persee;

class TcpConnectionHandler {

  public:

    typedef std::function<void(int)> StreamReqResolver;
    typedef std::function<void(char, StreamReqResolver)> StreamReqHandler;

  public:
    void setup(int port) {
      TCP.setup(port);
    }

    void update() {
      for(int i = 0; i < TCP.getLastID(); i++){ // getLastID is UID of all clients
        if( TCP.isClientConnected(i) ) { // check and see if it's still around

          if(TCP.getNumReceivedBytes(i) == 1) {
            char cmd;
            TCP.receiveRawBytes(i, &cmd, 1);

            switch(cmd){

              case CMD_GET_DEPTH_STREAM: {
                if(streamRequestHandler)
                  streamRequestHandler(CMD_GET_DEPTH_STREAM, [this, i](int streamPort){
                    if(streamPort > 0) {
                      sendStreamPortResponse(i, streamPort);
                    } else {
                      sendStreamFailureResponse(i);
                    }

                    TCP.disconnectClient(i);
                  });

                break;
              }

              case CMD_GET_COLOR_STREAM: {
                if(streamRequestHandler)
                  streamRequestHandler(CMD_GET_COLOR_STREAM, [this, i](int streamPort){
                    if(streamPort > 0) {
                      sendStreamPortResponse(i, streamPort);
                    } else {
                      sendStreamFailureResponse(i);
                    }

                    TCP.disconnectClient(i);
                  });

                break;
              }
            }

          }
        }
      }
    }

    void draw() {
      ofDrawBitmapString("Number of clients on tcpserver: " + ofToString(TCP.getNumClients()), 10, 20);
    }

    void setStreamRequestHandler(StreamReqHandler func){ streamRequestHandler = func; }

  private:

    void sendStreamPortResponse(int cid, int streamPort) {
      char buffer[5];
      // header
      buffer[0] = CMD_OK;
      // 4-byte int: port number
      buffer[1] = (char)((streamPort >> 24) & 0x0ff);
      buffer[2] = (char)((streamPort >> 16) & 0x0ff);
      buffer[3] = (char)((streamPort >> 8) & 0x0ff);
      buffer[4] = (char)(streamPort & 0x0ff);
      TCP.sendRawBytes(cid, buffer, 5);
    }

    void sendStreamFailureResponse(int cid){
      char cmd = CMD_ERROR;
      TCP.sendRawBytes(cid, &cmd, 1);
    }

  private:
    ofxTCPServer TCP;
    StreamReqHandler streamRequestHandler=nullptr;
};


class StreamSender;
typedef std::shared_ptr<StreamSender> StreamSenderRef;

class StreamSender {
  public:
    typedef std::function<void(StreamSenderRef)> StartedCallback;

  public:

    static StreamSenderRef create(int startPort, StartedCallback func) {
      auto sender = std::make_shared<StreamSender>(startPort);
      func(sender);
      return sender;
    }

    StreamSender(int port){
      TCP.setup(port);
    }

    void send(void* data, size_t size){
      TCP.sendRawBytesToAll((char*)data, size);
    }

    bool isActive() { return true; }
    int getPort() { return TCP.getPort(); }

  private:
    ofxTCPServer TCP;
};


class ofApp : public ofBaseApp{

  public: // methods
    ofApp(int argc, char** argv) : argc(argc), argv(argv) {}
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
    float fps=1.5f;
    float frameTime;
    bool resendFrames=true;
    int argc;
    char** argv;
    uint64_t nextFrameTime=0;
    int startPort = 4444;


    TcpConnectionHandler tcpConnectionHandler;
    CamInterface camInterface;
    Formatter formatter;
    std::shared_ptr<Compressor> compressor;
    std::vector<StreamSenderRef> colorSenders;
    std::vector<StreamSenderRef> depthSenders;
};

void ofApp::setup() {
  frameTime = 1.0f/fps;
  compressor = std::make_shared<Compressor>();

  startPort = argc > 2 ? std::atoi(argv[2]) : startPort;
  tcpConnectionHandler.setup(startPort);
  startPort+=1;

  tcpConnectionHandler.setStreamRequestHandler([this](char streamType, TcpConnectionHandler::StreamReqResolver resolver){
    if(streamType == CMD_GET_DEPTH_STREAM) {
      StreamSender::create(startPort, [this, resolver](StreamSenderRef sender){
        if(sender->isActive()) {
          resolver(sender->getPort());
          depthSenders.push_back(sender);
        } else {
          resolver(0);
        }
      });

      startPort+=1;
    }

    if(streamType == CMD_GET_COLOR_STREAM) {
      StreamSender::create(startPort, [this, resolver](StreamSenderRef sender) {
        if(sender->isActive()) {
          resolver(sender->getPort());
          colorSenders.push_back(sender);
        } else {
          resolver(0);
        }
      });

      startPort+=1;
    }
  });
}

void ofApp::update() {
  // send images to all active stream connections

  auto t = ofGetElapsedTimeMillis();
  if(t >= nextFrameTime) {
    nextFrameTime = t + frameTime;

    if((camInterface.getDepthListener().hasNew() || resendFrames) ) {

      // formatter.process(*depth);
      //
      // if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
      //   for(auto t : depthStreamTransmitters) {
      //     t->transmitFrame((const char*)compressor->getData(), compressor->getSize());
      //     std::cout << "sent " << compressor->getSize() << "-byte depth frame" << std::endl;
      //   }
      // }
    }

    if(camInterface.getColorListener().hasNew() || resendFrames) {

    }
  }
}

void ofApp::draw() {
  ofBackground(0);
  tcpConnectionHandler.draw();

  ofDrawBitmapString("depth senders: "+ofToString(depthSenders.size()), 10, 40);
  ofDrawBitmapString("color senders: "+ofToString(colorSenders.size()), 10, 60);
}

void ofApp::keyPressed(int key) {
  if(key == OF_KEY_ESC)
    ofExit();
}

//========================================================================

int main(int argc, char** argv){
  ofSetupOpenGL(800, 600, OF_WINDOW);
  ofRunApp(new ofApp(argc, argv));
}
