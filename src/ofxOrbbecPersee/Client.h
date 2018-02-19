#pragma once

#include "persee/Receiver.h"
#include "persee/Inflater.h"
#include "ImageStream.h"

namespace ofxOrbbecPersee {
  class Client {
    public: // types & consts

      struct Options {
        static const int DEFAULT_PORT = 4444;
        std::string host;
        int port;

        // Options(const std::string& host) : host(host), port(DEFAULT_PORT){}
        Options& setHost(const std::string& host) {
          this->host = host;
          return *this;
        }
      };

    public:
      void setup(const std::string &host) {
        this->setup(Options().setHost(host));
      }

      void setup(const Options& options);

      ImageStream& getDepthStream() {
        if(!this->depthStreamRef)
          this->depthStreamRef = std::make_shared<ImageStream>();
        return *this->depthStreamRef;
      }

    private:
      persee::Receiver receiver;
      persee::Inflater inflater;
      ImageStreamRef depthStreamRef = nullptr;
  };
}
