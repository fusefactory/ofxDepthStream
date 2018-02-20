#pragma once

#include "ImageStream.h"
#include "DepthStream.h"

namespace ofxOrbbecPersee {
  class Client {
    public: // types & consts

      struct Options {
        static const int DEFAULT_PORT = 4444;
        // static const std::string DEFAULT_HOST = "192.168.1.172";

        std::string host;
        int port = DEFAULT_PORT;

        Options& setHost(const std::string& host) { this->host = host; return *this; }
      };

      typedef std::function<void(persee::Receiver&)> QueuedRequestFunc;

    public:

      void setup(const std::string &host) {
        this->setup(Options().setHost(host));
      }

      void setup(const Options& options);
      // void update();

      // DepthStream& getDepthStream() {
      //   if(!this->depthStreamRef)
      //     this->depthStreamRef = std::make_shared<DepthStream>();
      //   return *this->depthStreamRef;
      // }

      DepthStreamRef createDepthStream();

    protected:
      void performQueuedRequest(QueuedRequestFunc func);
      void processRequestQueue();
      void processRequestQueueItem(QueuedRequestFunc func);

    private:
      Options opts;
      persee::ReceiverRef receiverRef=nullptr;
      std::vector<QueuedRequestFunc> requestQueue;
      bool bProcessingRequestQueue=false;

      // DepthStreamRef depthStreamRef = nullptr;
  };
}
