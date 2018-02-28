#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "Playback.h"

using namespace persee;

void Playback::start(const std::string& name) {
  filename = name;
  infile = new std::ifstream(filename, std::ofstream::binary);

  frameCount=0;
  bPlaying=true;
  startTime = std::chrono::steady_clock::now();

  std::cout << "started playback of: " << filename << std::endl;
  this->update();
}

void Playback::startThreaded() {
  this->thread = new std::thread(std::bind(&Playback::threadFunc, this));
}

void Playback::startThreaded(const std::string& name){
  filename = name;
  this->startThreaded();
}

void Playback::stop(bool wait) {
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

bool Playback::update(FrameCallback inlineCallback) {
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
    auto ms = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();

    if(ms > nextFrame->time) {
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

Playback::Frame* Playback::readFrame() {
  if(infile->read((char*)&frame.time, sizeof(uint32_t)) &&
    infile->read((char*)&frame.size, sizeof(uint32_t)) &&
    infile->read((char*)&frame.buffer, frame.size)) {
    return &frame;
  }

  return NULL;
}

void Playback::threadFunc() {
  this->start(this->filename);
  while(this->bPlaying){
    this->update();
    std::this_thread::sleep_for(std::chrono::duration<long, std::milli>(50l));
  }
}

void Playback::onEnd() {
  stop(false);
  if(bLoop) {
    if(frameCount == 0) {
      std::cout << "[persee::Playback] file appears empty" << std::endl;
    } else {
      start(this->filename);
    }
  }
}
