#include "ofxUnitTests.h"
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"

class ofApp: public ofxUnitTestsApp{

    void run(){
      test_eq(1+1, 2, "");
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
