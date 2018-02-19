#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr

#include <netdb.h> //hostent
#include <functional>
#include <math.h>

#include "OniSampleUtilities.h"
#include "Transmitter.h"

using namespace persee;

Transmitter::Transmitter(int port) : port(port) {
  this->thread = new std::thread(std::bind(&Transmitter::serverThread, this));
}

Transmitter::~Transmitter() {
  if(this->thread) {
    std::cout << "stopping server thread";
    bRunning = false;
    thread->join();
    std::cout << "server thread done";
    delete thread;
  }
}

bool Transmitter::transmit(char* data, size_t size) {
  std::string ss(data, size);
  // std::cout << "should transmit " << size << " bytes on port " << port << ": " << ss << std::endl;

  if (!bConnected) {
    // std::cout << "no client, didn't sent " << size << " bytes." << std::endl;
    return false;
  }

  // transmittion-header; 4-byte package length
  char header[4];
  header[0] = (char)((size >> 24) & 0x0ff);
  header[1] = (char)((size >> 16) & 0x0ff);
  header[2] = (char)((size >> 8) & 0x0ff);
  header[3] = (char)(size & 0x0ff);
  auto n = write(newsockfd,header,4);

  if (n < 0) {
    error("ERROR writing package-header to socket");
    return false;
  }

  // content
  n = write(newsockfd,data,size);

  if (n < 0) {
    error("ERROR writing package content to socket");
    return false;
  }

  // std::cout << "sent " << size << " bytes." << std::endl;
  return true;
}


bool Transmitter::start() {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int option=1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (sockfd < 0) {
     error("ERROR opening socket");
     return false;
   }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = this->port;//atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     error("ERROR on binding");
     return false;
   }

  listen(sockfd, 5);
  return true;
}

void Transmitter::serverThread() {
  int n;

  while(bRunning) {
    if(this->start()) {
      clilen = sizeof(cli_addr);

      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) {
           error("ERROR on accept");
      } else {
        bConnected = true;
        std::cout << "client connected" << std::endl;

        while(bRunning){
          n=recv(newsockfd,packet,MAXPACKETSIZE-1,0);
          if(n==0) break;
          Sleep(100);
          // msg[n]=0;
          //send(newsockfd,msg,n,0);
          // Message = string(msg);
        }

        std::cout << "client disconnected" << std::endl;
        bConnected = false;
      }

      close(newsockfd);
    }

    close(sockfd);
    Sleep(1000);
  }
}
