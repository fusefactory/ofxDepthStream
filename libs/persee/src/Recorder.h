#pragma once

#include <chrono>
#include "Buffer.h"

namespace persee {
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

    protected:

      bool record(const void* data, uint32_t size);

    private:
      std::chrono::steady_clock::time_point startTime;
      std::ofstream* outfile;
      std::ostream* ostream;
      size_t frameCount=0;
      size_t byteCount=0;
      bool bVerbose=true;
  };
}
