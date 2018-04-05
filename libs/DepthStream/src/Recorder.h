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

#pragma once

#include <chrono>
#include "Buffer.h"

namespace depth {
  class Recorder : public Buffer  {

    public:

      // Recorder(){}
      ~Recorder();
      void start(const std::string& name);
      void start(std::ostream& ostream);
      void stop();

      // Buffer interface; override Buffer::write and add recording logic
      virtual void write(const void* data, size_t size) override {
        this->record(data, size);

        // call parent logic (forward data to any target set using our
        // Buffer::setOutputTo method)
        Buffer::write(data, size);
      }

      bool isRecording() {
        return this->ostream != NULL;
      }

    protected:

      bool record(const void* data, uint32_t size);

    private:
      std::chrono::steady_clock::time_point startTime;
      std::ofstream* outfile;
      std::ostream* ostream = NULL;
      size_t frameCount=0;
      size_t byteCount=0;
      bool bVerbose=true;
  };
}
