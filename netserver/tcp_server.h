#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

#define MAXPACKETSIZE 8192
#define MAX_CLIENT 1000

using RecvCallBack =
    add_pointer<void(int sockfd, char *buffer, int size)>::type;

class TCPServer {
public:
  TCPServer() = default;
  virtual ~TCPServer();

  int Send(int cilent_socket, char *buff, int size);
  int Process(int port);

  void RegisterRecvCallBack(RecvCallBack cb) { callback_ = cb; }
  void UnRegisterRecvCallBack() { callback_ = nullptr; }

private:
  void Accepted();
  int Recvieve(int cilent_socket);

private:
  int sockfd;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  vector<shared_ptr<thread>> accept_threads_;
  vector<shared_ptr<thread>> recv_threads_;
  RecvCallBack callback_;
};

#endif
