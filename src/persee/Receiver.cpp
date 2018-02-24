#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr

#include<netdb.h> //hostent
#include <functional>
#include <math.h>
#include "zlib.h"

#include "OniSampleUtilities.h"
#include "Receiver.h"

using namespace persee;

Receiver::~Receiver() {
  this->stop(true);
}

void Receiver::start(const std::string& host, int port) {
  this->stop(true); // calling start while running will cause restart
  this->host = host;
  this->port = port;
  bRunning = true;
  // start thread
  this->thread = new std::thread(std::bind(&Receiver::threadFunc, this));
}

void Receiver::stop(bool wait){
  // this->cout() << "stopping client thread...";
  bRunning = false;
  this->disconnect();

  if(wait) {

    if(this->thread) {
      std::cout << "[persee::Receiver] waiting for thread to end" << std::endl;
      thread->join();
      // this->cout() << " done" << std::endl;
      delete thread;
      thread=NULL;
    }
  }
}

void Receiver::error(const char *msg) {
    perror(msg);
    // exit(1);
}

void Receiver::threadFunc() {
  // connect/reconnect loop
  while (bRunning) {
    if(!bConnected) {
      if(!this->connectToServer(this->host, this->port))
        Sleep(connectAttemptInterval);
    }

    if(bConnected) {
      // get 4-byte header
      if(this->receive(4)) {
        int b0 = (int)(0x0ff & buffer[0]);
        int b1 = (int)(0x0ff & buffer[1]);
        int b2 = (int)(0x0ff & buffer[2]);
        int b3 = (int)(0x0ff & buffer[3]);
        int total = ((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
        if(bVerbose) this->cout() << "got header for " << total << " bytes" << std::endl;
        int alreadyGot = this->recvSize - 4;

        // get X-byte package
        if(this->receive(this->buffer + 4 + alreadyGot, total - alreadyGot)) {
          // this->cout() << "received: " << total << " byte-package" << std::endl;
          this->bHasNew = true;
          this->lastPackageSize = total;

          this->take(this->buffer+4, total); // our persee::Buffer interface

          if(frameCallback)
            frameCallback((const void*)(this->buffer+4), total);
        }
      }
    }

    Sleep(cycleSleep);
  }
}

bool Receiver::connectToServer(const std::string& address, int port) {
  //create socket if it is not already created
  // if(sock == -1)
  // {
      //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
  {
      error("Could not create socket");
  }

  // std::cout<<"Socket created\n";
  // }
  // else    {   /* OK , nothing */  }

  // memset(&server, '\0', sizeof(server));
  //setup address structure
  if(inet_addr(address.c_str()) == INADDR_NONE)
  {
      struct hostent *he;
      struct in_addr **addr_list;

      //resolve the hostname, its not an ip address
      if ( (he = gethostbyname( address.c_str() ) ) == NULL)
      {
          //gethostbyname failed
          error("gethostbyname");
          std::cout<<"Failed to resolve hostname\n";

          return false;
      }

      //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
      addr_list = (struct in_addr **) he->h_addr_list;

      for(int i = 0; addr_list[i] != NULL; i++)
      {
          //strcpy(ip , inet_ntoa(*addr_list[i]) );
          server.sin_addr = *addr_list[i];

          std::cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i]) << std::endl;

          break;
      }
  }

  //plain ip address
  else
  {
      server.sin_addr.s_addr = inet_addr( address.c_str() );
  }

  server.sin_family = AF_INET;
  server.sin_port = htons( port );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
      // perror("connect failed. Error");
      this->cout() << "Failed to connect to " << address << ":" << port << std::endl;
      return false;
  }

  this->cout() << "connected to " << address << ":" << port << std::endl;
  bConnected=true;
  return true;
}

void Receiver::disconnect(){
  close(sock);

  if(bConnected) {
    bConnected=false;
    this->cout() << "disconnected from " << this->host << ":" << this->port << std::endl;
  }
}

bool Receiver::receive(size_t size){
  if(size > BUF_SIZE) {
    std::cerr << "Requested recveive-size larger than buffer" << std::endl;
    return false;
  }

  return this->receive(this->buffer, size);
}

bool Receiver::receive(char* buffer, size_t size) {
  // if(size > bufferSize){
  //   if(buffer){
  //     delete buffer;
  //     buffer = new char[size];
  //     bufferSize = size;
  //   }
  // }

  size_t amount=0;

  while(amount < size) {
    int packageSize = recv(sock, buffer+amount, size-amount, 0);

    if(packageSize < 0) {
      // this->disconnect();
      return false;
    }

    // if(packageSize <= 0) {
    //
    //   // perror("recv failed");
    //   return false;
    // }

    amount += packageSize;
  }

  this->recvSize = amount;
  return true;
}

bool Receiver::send_data(std::string data) {
    this->cout() << "Sending data...";
    //Send some data
    if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    std::cout<<"Sent\n";

    return true;
}
