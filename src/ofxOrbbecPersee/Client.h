#pragma once

#include "DepthStream.h"
#include "ColorStream.h"

namespace ofxOrbbecPersee {
  class Client {
    public: // types & consts

      struct Options {
        static const int DEFAULT_PORT = 4444;
        // static const std::string DEFAULT_HOST = "192.168.1.172";

        std::string host;
        int port = DEFAULT_PORT;

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

      // void setup(const Options& options);
      // void update();

      // DepthStream& getDepthStream() {
      //   if(!this->depthStreamRef)
      //     this->depthStreamRef = std::make_shared<DepthStream>();
      //   return *this->depthStreamRef;
      // }

      DepthStreamRef createDepthStream();
      ColorStreamRef createColorStream();

    protected:
      // void performQueuedRequest(QueuedRequestFunc func);
      // void processRequestQueue();
      // // void processRequestQueueItem(QueuedRequestFunc func);
      //
      // bool requestStreamComms(persee::Receiver& recvr, char cmd, ImageStreamRef stream);

    private:
      Options opts;
      // persee::ReceiverRef receiverRef=nullptr;
      // std::vector<QueuedRequestFunc> requestQueue;
      // bool bProcessingRequestQueue=false;
      // int requestAttemptCount=0;
      // static const int MAX_REQUEST_ATTEMPTS=5;
      // static const int REQUEST_RETRY_DELAY=500; // ms
      // static const int REQUEST_IDLE_DELAY=2000; // ms
      // DepthStreamRef depthStreamRef = nullptr;
  };
}
