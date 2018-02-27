#pragma once

#include <chrono>
#include "Buffer.h"

namespace persee {
  class Playback : public Buffer {

    public:

      typedef std::function<void(void*, size_t)> FrameCallback;

    public:

      struct Frame {
        static const size_t BUF_SIZE=(1280*720*3);
        char buffer[BUF_SIZE];
        uint32_t size, time;
      };

      ~Playback(){
        stop(true);
      }

      void start(const std::string& name) {
        filename = name;
        infile = new std::ifstream(filename, std::ofstream::binary);

        frameCount=0;
        bPlaying=true;
        startTime = ofGetElapsedTimeMillis();

        std::cout << "started playback of: " << filename << std::endl;
        this->update();
      }

      void startThreaded() {
        this->thread = new std::thread(std::bind(&Playback::threadFunc, this));
      }

      void stop(bool wait=true) {
        bPlaying=false;

        if(infile){
          infile->close();
          delete infile;
          infile=NULL;
        }

        if(wait){
          if(thread) {
            thread->join();
            delete thread;
            thread=NULL;
          }
        }
      }

      bool update(FrameCallback inlineCallback=nullptr) {
        if(!bPlaying) return false;

        if(!nextFrame){
          nextFrame=readFrame();

          if(!nextFrame){
            bPlaying=false;
            this->onEnd();
            return false;
          }
        }

        if(nextFrame) {
          auto t = ofGetElapsedTimeMillis() - startTime;

          if(t > nextFrame->time) {
            if(inlineCallback)
              inlineCallback(nextFrame->buffer, nextFrame->size);

            // implement/execute our Buffer interface
            Buffer::write(nextFrame->buffer, nextFrame->size);

            nextFrame = NULL;
            frameCount += 1;
            return true;
          }
        }

        return false;
      }

    protected:

      Frame* readFrame() {
        if(infile->read((char*)&frame.time, sizeof(uint32_t)) &&
          infile->read((char*)&frame.size, sizeof(uint32_t)) &&
          infile->read((char*)&frame.buffer, frame.size)) {
          return &frame;
        }

        return NULL;
      }

      void threadFunc() {
        this->start(this->filename);
        while(this->bPlaying){
          this->update();
          std::this_thread::sleep_for(std::chrono::duration<long, std::milli>(50l));
        }
      }

      void onEnd() {
        stop(false);
        if(bLoop) {
          if(frameCount == 0) {
            std::cout << "[persee::Playback] file appears empty" << std::endl;
          } else {
            start(this->filename);
          }
        }
      }

    private:
      bool bPlaying=false;
      bool bLoop=true;
      uint64_t startTime;

      std::string filename;
      std::ifstream* infile;
      // size_t frameCount=0;
      Frame frame;
      size_t frameCount;
      Frame* nextFrame=NULL;

      std::thread* thread=NULL;
  };
}
