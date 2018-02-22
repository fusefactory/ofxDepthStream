#pragma once

// stdlib
#include "zlib.h"
// of
#include "ofMain.h"
// ofxaddons
#include "ofxTCPClient.h"
// local livs
#include "ImageStream.h"
#include "persee/Receiver.h"
#include "persee/Inflater.h"
#include "persee/protocol.h"

namespace ofxOrbbecPersee {
  class StreamReceiver : public ImageStream::Addon, public ofThread {

    public:

      StreamReceiver(ImageStream* stream, const std::string& addr, int port) : ImageStream::Addon(stream), address(addr), port(port) {
      }

      void start() {
        this->startThread();
      }

      void stop() {
        stopThread();
      }

      virtual void update() {
        if(newData) {
          // ofLogNotice() << "offering new data to ImageStream";
          this->getImageStream()->offerData((char*)newData, newDataSize);
          newData = NULL;
        }
      }

    protected:

      bool receiveRaw(char* buffer, size_t size){
        int packetSize, ttl = 0;

        while(tcpClient.isConnected() && ttl < size){
          packetSize = tcpClient.receiveRawBytes(buffer+ttl, size-ttl);
          if(packetSize < 0)
            return false;

          ttl += packetSize;
        }

        return true;
      }

      bool recvInt(int& target) {
        char buffer[4];

        if(!this->receiveRaw(buffer, 4))
          return false;

        int b0 = (int)(0x0ff & ((char*)buffer)[0]);
        int b1 = (int)(0x0ff & ((char*)buffer)[1]);
        int b2 = (int)(0x0ff & ((char*)buffer)[2]);
        int b3 = (int)(0x0ff & ((char*)buffer)[3]);
        target = ((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
        return true;
      }

      char *decompress(char *compressedBytes, unsigned int length) {

          unsigned full_length = length ;
          unsigned half_length = length / 2;

          unsigned uncompLength = full_length ;
          char *uncompressedBytes = (char *) calloc(sizeof(char), uncompLength);

          if (length == 0) {
              uncompressedBytes = compressedBytes;
              return uncompressedBytes ;
          }

          z_stream strm;
          strm.next_in = (Bytef *)compressedBytes;
          strm.avail_in = length ;
          strm.total_out = 0;
          strm.zalloc = Z_NULL;
          strm.zfree = Z_NULL;

          bool done = false ;

          if (inflateInit2(&strm, MAX_WBITS) != Z_OK) {
              free(uncompressedBytes);
              return uncompressedBytes;
          }

          while (!done) {
              // if our output buffer is too small
              if (strm.total_out >= uncompLength ) {
                  // Increase size of output buffer
                  char *uncomp2 = (char *) calloc( sizeof(char), uncompLength + half_length);
                  memcpy(uncomp2, uncompressedBytes, uncompLength);
                  uncompLength += half_length;
                  free(uncompressedBytes);
                  uncompressedBytes = uncomp2;
              }

              strm.next_out = (Bytef *) (uncompressedBytes + strm.total_out);
              strm.avail_out = uncompLength - strm.total_out;

              // inflate another chunk
              int err = inflate (& strm, Z_SYNC_FLUSH);
              if (err == Z_STREAM_END) done = true;
              else if (err != Z_OK)  {
                  break;
              }
          }

          if (inflateEnd (& strm) != Z_OK) {
              free(uncompressedBytes);
              return uncompressedBytes;
          }

          return uncompressedBytes;
      }

      virtual void threadedFunction() {
        while (isThreadRunning()) {

          // connect
          if (!tcpClient.isConnected()) {

            auto t = ofGetElapsedTimeMillis();

            if(t >= nextConnectTime){
              ofLogNotice() << "[StreamReceiver] " << ofGetTimestampString() + ": connecting to " + address + ":" + to_string(port);

              tcpClient.setup(address, port);
              nextConnectTime = t + connectAttemptInterval;

              if (tcpClient.isConnected()) {
                  ofLogNotice() << "[StreamReceiver] " << ofGetTimestampString() + ": connected to " + address + ":" + to_string(port);
              }
            }
          }

          // read
          if (tcpClient.isConnected()) {
            int headerSize;

            // read header (4-byte body-length integer)
            if(recvInt(headerSize) && headerSize > 0){
              // ofLogNotice() << "Got Header size: " << headerSize;

              // data
              char tmpBuf[headerSize];
              receiveRaw(tmpBuf, headerSize);

              // auto bufr = decompress(tmpBuf, headerSize);
              // if(newData) free(newData);
              // newData = bufr;

              if(inflater.inflate(tmpBuf, headerSize)) {
                newData = (void*)inflater.getData();
                newDataSize = inflater.getSize();
              } else {
                ofLogWarning() << "[StreamReceiver] inflate failed";
              }
            }
          }

          sleep(cycleSleep);
        } // while running
      }

      virtual void destroy(){
        stop();
      };

    private:
      std::string address;
      int port; // http port; where to request stream from, NOT the stream port

      ofxTCPClient tcpClient;
      persee::Inflater inflater;

      // buffer, etc.
      static const int BUF_SIZE = (1280&720*3+10);
      char buffer[BUF_SIZE];

      void* newData=NULL;
      size_t newDataSize;

      // timings
      int connectAttemptInterval=3000;
      int nextConnectTime=0;
      int cycleSleep = 10;
  };
}
