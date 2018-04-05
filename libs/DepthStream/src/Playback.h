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

#pragma once

#include <chrono>
#include "Buffer.h"

namespace depth {
  class Playback : public Buffer {

    public: // types

      typedef std::function<void(void*, size_t)> FrameCallback;

      struct Frame {
        uint32_t time, size;
        static const size_t BUF_SIZE=(1280*720*3);
        char buffer[BUF_SIZE];
      };

    public: // methods

      ~Playback(){
        stop(true /* wait for thread -if any- to finish */);
      }

      void start(const std::string& name);
      void startThreaded();
      void startThreaded(const std::string& name);
      void stop(bool wait=true);
      bool update(FrameCallback inlineCallback=nullptr);

      bool isPlaying() const { return bPlaying; }
      const std::string& getFilename() const { return filename; }

    protected: // methods

      Frame* readFrame();
      void threadFunc();
      void onEnd();

    private: // attributes

      bool bPlaying=false;
      bool bLoop=true;
      std::chrono::steady_clock::time_point startTime;

      std::string filename;
      std::ifstream* infile;

      size_t frameCount;
      Frame frame; // our read-buffer
      Frame* nextFrame=NULL;

      std::thread* thread=NULL;
  };
}
