//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  This file is part of ofxDepthStream.
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

#include <string>
#include <iostream>
#include <fstream>
#include "Recorder.h"

using namespace std;
using namespace depth;

Recorder::~Recorder() {
  stop();
}

void Recorder::start(const std::string& name) {
  stop();
  outfile = new std::ofstream(name, std::ofstream::binary);
  this->start(*outfile);
  if(bVerbose) std::cout << "started recording to: " << name << std::endl;
}

void Recorder::start(std::ostream& ostream) {
  this->ostream = &ostream;
  frameCount = 0;
  byteCount = 0;
  startTime = std::chrono::steady_clock::now();
}

void Recorder::stop() {
  ostream = NULL;

  if(outfile){
    outfile->close();
    delete outfile;
    outfile=NULL;
    if(bVerbose) std::cout << "Recording stopped; recorded " << frameCount << " frames containing " << byteCount << " bytes of image data" << std::endl;
  }
}

bool Recorder::record(const void* data, uint32_t size) {
  if(!ostream)
    return false;

  auto timestamp = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
  ostream->write((const char*)&timestamp, sizeof(timestamp)); // 4 byte timestamp
  ostream->write((const char*)&size, sizeof(size)); // 4 byte frame size
  ostream->write((const char*)data, size); // frame body
  frameCount+=1;
  byteCount+=size;
  return true;
}
