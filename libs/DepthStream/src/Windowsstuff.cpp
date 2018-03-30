#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <stdio.h>
#include "Windowsstuff.h"

bool bWinSocksAreInitialized=false;

bool depth::makeSureWindowSocketsAreInitialized() {
  if (bWinSocksAreInitialized) return true;

  WSADATA wsaData;
  WORD wsaVersionReq = MAKEWORD(2, 2); // this the right version?
  int err = WSAStartup(wsaVersionReq, &wsaData);

  if (err != 0) {
    printf("WSAStartup failed with error: %d\n", err);
    return false;
  }

  bWinSocksAreInitialized = true;
  return true;
}
#endif