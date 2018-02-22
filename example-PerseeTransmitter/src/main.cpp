// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
#include "ofxTCPServer.h"
// local
#include "persee/protocol.h"
#include "persee/CamInterface.h" //
#include "persee/Formatter.h"
#include "persee/Compressor.h"

using namespace persee;

// class TcpConnectionHandler {
//
//   public:
//
//     typedef std::function<void(int)> StreamReqResolver;
//     typedef std::function<void(char, StreamReqResolver)> StreamReqHandler;
//
//   public:
//
//     void setup(int port) {
//       TCP.setup(port);
//     }
//
//     void update() {
//       for(int i = 0; i <= TCP.getLastID(); i++){ // getLastID is UID of all clients
//         if( TCP.isClientConnected(i) ) { // check and see if it's still around
//           char cmd;
//           TCP.receiveRawBytes(i, &cmd, 1);
//           ofLogNotice() << "Got CMD: " << cmd;
//           switch(cmd){
//
//             case CMD_GET_DEPTH_STREAM: {
//               if(streamRequestHandler)
//                 streamRequestHandler(CMD_GET_DEPTH_STREAM, [this, i](int streamPort){
//                   if(streamPort > 0) {
//                     sendStreamPortResponse(i, streamPort);
//                   } else {
//                     sendStreamFailureResponse(i);
//                   }
//
//                   TCP.disconnectClient(i);
//                 });
//
//               break;
//             }
//
//             case CMD_GET_COLOR_STREAM: {
//               if(streamRequestHandler)
//                 streamRequestHandler(CMD_GET_COLOR_STREAM, [this, i](int streamPort){
//                   if(streamPort > 0) {
//                     sendStreamPortResponse(i, streamPort);
//                   } else {
//                     sendStreamFailureResponse(i);
//                   }
//
//                   TCP.disconnectClient(i);
//                 });
//
//               break;
//             }
//
//             default: {
//
//               ofLogWarning() << "Got UNKNOWN CMD-byte from new client connection:" << cmd;
//               TCP.disconnectClient(i);
//             }
//           }
//         }
//       }
//     }
//
//     void draw() {
//       ofDrawBitmapString("Number of clients on tcpserver: " + ofToString(TCP.getNumClients()), 10, 20);
//     }
//
//     void setStreamRequestHandler(StreamReqHandler func){ streamRequestHandler = func; }
//
//   private:
//
//     void sendStreamPortResponse(int cid, int streamPort) {
//       char buffer[5];
//       // header
//       buffer[0] = CMD_OK;
//       // 4-byte int: port number
//       buffer[1] = (char)((streamPort >> 24) & 0x0ff);
//       buffer[2] = (char)((streamPort >> 16) & 0x0ff);
//       buffer[3] = (char)((streamPort >> 8) & 0x0ff);
//       buffer[4] = (char)(streamPort & 0x0ff);
//       TCP.sendRawBytes(cid, buffer, 5);
//     }
//
//     void sendStreamFailureResponse(int cid){
//       char cmd = CMD_ERROR;
//       TCP.sendRawBytes(cid, &cmd, 1);
//     }
//
//   private:
//     ofxTCPServer TCP;
//     StreamReqHandler streamRequestHandler=nullptr;
// };
//

class StreamSender;
typedef std::shared_ptr<StreamSender> StreamSenderRef;

class StreamSender {

  public:

    typedef std::function<void(StreamSenderRef)> StartedCallback;
    typedef std::function<void(StreamSender*)> ClosedCallback;

  public:

    static StreamSenderRef create(int startPort) {
      return std::make_shared<StreamSender>(startPort);
    }

    StreamSender(int port){
      TCP.setup(port);
    }

    bool sendFrame(const void* data, size_t size){
      int connected=false;

      for(int i = 0; i <= TCP.getLastID(); i++){ // getLastID is UID of all clients
        if( TCP.isClientConnected(i) ) { // check and see if it's still around
          connected=true;
        }
      }

      bConnected = connected;
      if(connected) {
        if(!bHasHadConnection) ofLogNotice() << "New Connection on StreamSender";
        bHasHadConnection=true;

      } else
        return false;

      char buffer[4];
      formatInt(buffer, size);
      // header
      TCP.sendRawBytesToAll((const char*)buffer, 4);
      // body
      this->sendBig(data,size);
      return true;
    }

    bool isDead() { return bHasHadConnection && !bConnected; }
    int getPort() { return TCP.getPort(); }

    void setClosedCallback(ClosedCallback func) { closedCallback = func; }

  protected:

    bool sendBig(const void* data, size_t size){
      size_t ttl = 0;
      size_t messageSize = 256;

      while( ttl < size ) {
        int curSize = std::min(messageSize, (int)size-ttl);
        if(!TCP.sendRawBytesToAll((const char*)data+ttl, curSize))
          return false;
        ttl += curSize;
      }

      return true;
    }

    void formatInt(char* buffer, int value){
      // transmittion-header; 4-byte package length
      buffer[0] = (char)((value >> 24) & 0x0ff);
      buffer[1] = (char)((value >> 16) & 0x0ff);
      buffer[2] = (char)((value >> 8) & 0x0ff);
      buffer[3] = (char)(value & 0x0ff);
    }

  private:
    ofxTCPServer TCP;
    ClosedCallback closedCallback;

    bool bConnected=false;
    bool bHasHadConnection=false;
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


    // TcpConnectionHandler tcpConnectionHandler;
    CamInterface camInterface;
    Formatter formatter;
    std::shared_ptr<Compressor> compressor;
    std::vector<StreamSenderRef> colorSenders;
    std::vector<StreamSenderRef> depthSenders;
};

void ofApp::setup() {
  ofSetWindowShape(300,200);

  int depthPort = argc > 1 ? atoi(argv[1]) : 4445;
  int colorPort = argc > 2 ? atoi(argv[2]) : 4446;
  // int commPort = argc > 3 ? atoi(argv[3]) : 4447;

  auto sender = StreamSender::create(depthPort);
  depthSenders.push_back(sender);
  ofLogNotice() << "DEPTH Sender created";

  sender = StreamSender::create(colorPort);
  colorSenders.push_back(sender);
  ofLogNotice() << "COLOR Sender created";

  // if(commPort != 0){
  //   tcpConnectionHandler.setup(commPort);
  //   tcpConnectionHandler.setStreamRequestHandler([this](char streamType, TcpConnectionHandler::StreamReqResolver resolver){
  //     ofLogNotice() << "got stream request";
  //     if(streamType == CMD_GET_DEPTH_STREAM) {
  //       ofLogNotice() << "got DEPTH stream request";
  //
  //       auto sender = StreamSender::create(startPort)
  //       ofLogNotice() << "DEPTH Streamer created";
  //       resolver(sender->getPort());
  //       depthSenders.push_back(sender);
  //       startPort+=1;
  //     }
  //
  //     if(streamType == CMD_GET_COLOR_STREAM) {
  //       ofLogNotice() << "got COLOR stream request";
  //       auto sendre = StreamSender::create(startPort);
  //       ofLogNotice() << "COLOR Streamer created";
  //       resolver(sender->getPort());
  //       colorSenders.push_back(sender);
  //       startPort+=1;
  //     }
  //   });
  //
  //   startPort = commPort+1;
  // }

  frameTime = 1.0f/fps*1000.0f;
  compressor = std::make_shared<Compressor>();
}

void ofApp::update() {
  // send images to all active stream connections
  // tcpConnectionHandler.update();

  auto t = ofGetElapsedTimeMillis();
  if(t >= nextFrameTime) {
    nextFrameTime = t + frameTime;

    if((camInterface.getDepthListener().hasNew() || resendFrames) && depthSenders.size() > 0) {
      formatter.process(*camInterface.getDepthStream());
      if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
        for(int idx=depthSenders.size()-1; idx>=0; idx--){
          auto t = depthSenders[idx];
          bool success = t->sendFrame((const char*)compressor->getData(), compressor->getSize());
          // if(!success && t->isDead()){
          //   ofLogNotice() << "DEPTH stream closed";
          //   depthSenders.erase(depthSenders.begin()+idx);
          // }
        }
      }
    }

    if(camInterface.getColorListener().hasNew() || resendFrames) {
      formatter.process(*camInterface.getColorStream());

      if(formatter.getData() && compressor->compress(formatter.getData(), formatter.getSize())) {
        for(int idx=colorSenders.size()-1; idx>=0; idx--){
          auto t = colorSenders[idx];
          bool success = t->sendFrame((const char*)compressor->getData(), compressor->getSize());
          // if(!success && t->isDead()){
          //   ofLogNotice() << "COLOR stream closed";
          //   colorSenders.erase(colorSenders.begin()+idx);
          // }
        }
      }
    }
  }
}

void ofApp::draw() {
  ofBackground(0);
  // tcpConnectionHandler.draw();
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
