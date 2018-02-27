#pragma once

#include <chrono>
#include "Buffer.h"

namespace persee {
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
      void stop(bool wait=true);
      bool update(FrameCallback inlineCallback=nullptr);

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
