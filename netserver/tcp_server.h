#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "logger/log.h"

using namespace std;

#define MAXPACKETSIZE 8192
#define MAX_CLIENT 1000

using RecvCallBack =
    add_pointer<void(int sockfd, char* buffer, int size)>::type;

class TCPServer {
   public:
    TCPServer()
        : sockfd(-1),
          quit_(0),
          serverAddress{0},
          clientAddress{0},
          callback_(nullptr){};
    virtual ~TCPServer();

    int Send(int cilent_socket, char* buff, int size);
    int Process(int port);

    void RegisterRecvCallBack(RecvCallBack cb) { callback_ = cb; }
    void UnRegisterRecvCallBack() { callback_ = nullptr; }
    void SaveExit();

   private:
    void Accepted();
    int Recvieve(int cilent_socket);

   private:
    int sockfd;
    int quit_;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    RecvCallBack callback_;
};

#endif
