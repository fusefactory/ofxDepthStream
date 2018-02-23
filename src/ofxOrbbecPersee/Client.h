#pragma once

#include "DepthStream.h"
#include "ColorStream.h"

namespace ofxOrbbecPersee {
  class Client {
    public: // types & consts

      struct Options {
        // static const std::string DEFAULT_HOST = "192.168.1.172";

        std::string host;
        int port = 4444;
        int depthStreamPort=4445;
        int colorStreamPort=4446;

        Options& setHost(const std::string& host) { this->host = host; return *this; }
        Options& setPort(int port) { this->port = port; return *this; }
      };

      // typedef std::function<bool(persee::Receiver&)> QueuedRequestFunc;

    public:

      void setup(const std::string &host) {
        this->opts = Options().setHost(host);
      }

      void setup(const std::string &host, int port) {
        this->opts = Options().setHost(host).setPort(port);
      }

      DepthStreamRef createDepthStream();
      ColorStreamRef createColorStream();

    protected:

    private:
      Options opts;

  };
}
