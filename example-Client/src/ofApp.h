// OF
#include "ofMain.h"
// addons
#include "ofxDepthStream/ofxDepthStream.h"


class ofApp : public ofBaseApp{

  public: // methods
    void setup() override;
    void update() override;
    void draw() override;

  private: // attributes
    std::string remoteCamAddress = "127.0.0.1"; // persee.local"; //"192.168.1.226"; // "127.0.0.1";
    int depthPort = 4445;
    // int colorPort = 4446;

    depth::ReceiverRef depthReceiverRef;
    // depth::ReceiverRef colorReceiverRef;
    depth::InflaterRef inflaterRef; // re-usable inflater instance for optimization

    ofTexture depthTex;
    // ofTexture colorTex;
};
