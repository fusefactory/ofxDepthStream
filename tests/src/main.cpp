//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

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
