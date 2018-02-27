#pragma once

#include "Buffer.h"

namespace persee {
  class Recorder : public Buffer  {

    public:

      Recorder(){
      }

      void start(const std::string& name) {
        outfile = new std::ofstream(name, std::ofstream::binary);

        frameCount = 0;
        byteCount = 0;
        startTime = ofGetElapsedTimeMillis();

        std::cout << "started recording to: " << name << std::endl;
      }

      void stop() {
        if(outfile){
          outfile->close();
          delete outfile;
          outfile=NULL;
          std::cout << "Recording stopped; recorded " << frameCount << " frames containing " << byteCount << " bytes of image data" << std::endl;
        }
      }

      bool record(const void* data, uint32_t size) {
        if(!outfile)
          return false;

        uint32_t t = ofGetElapsedTimeMillis() - startTime;
        outfile->write((const char*)&t, sizeof(t)); // 4 byte timestamp
        outfile->write((const char*)&size, sizeof(size)); // 4 byte frame size
        outfile->write((const char*)data, size); // frame body
        frameCount+=1;
        byteCount+=size;
        return true;
      }

      // Buffer interface
      virtual void write(const void* data, size_t size) override {
        this->record(data, size);

        // call parent logic (forward data to any target set using our
        // Buffer::setOutputTo method)
        Buffer::write(data, size);
      }

    private:
      uint64_t startTime;
      std::ofstream* outfile;
      size_t frameCount=0;
      size_t byteCount=0;
  };
}
