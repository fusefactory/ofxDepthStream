#include "ofxUnitTests.h"
#include "DepthStream.h"

class ofApp: public ofxUnitTestsApp{
  void run(){
    test_compress_inflate();
  }

  void test_compress_inflate() {
    // create frame ref
    auto ref1 = depth::Frame::ref(640*480*2);
    auto compressedRef = depth::compress(ref1);
    auto ref2 = depth::inflate(compressedRef);

    test_eq(ref1->size(), ref2->size(), "");
    test_eq(ref2->size(), (640*480*2), "");
    test_eq(memcmp((char*)ref1->data(), (char*)ref2->data(), ref1->size()), 0, "");
    test_eq(compressedRef->size() < ref1->size(), true, "");
  }
};

#include "ofAppNoWindow.h"
#include "ofAppRunner.h"

int main( ){
  ofInit();
  auto window = std::make_shared<ofAppNoWindow>();
  auto app = std::make_shared<ofApp>();
  ofRunApp(window, app);
  return ofRunMainLoop();
}
