#include "tcp_server.h"

#include <net/if.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

TCPServer::~TCPServer() {}

void TCPServer::SaveExit() { quit_ = true; }

int TCPServer::Send(int cilent_socket, char* buff, int size) { return send(cilent_socket, buff, size, 0); }

int TCPServer::Recvieve(int cilent_socket) {
  //LOG_INFO("TCPServer::Recvieve\n");
  char buffer[MAXPACKETSIZE];
  int size = sizeof(buffer);
  struct timeval interval = {3, 0};
  setsockopt(cilent_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(struct timeval));
  while (!quit_) {
    int length = recv(cilent_socket, buffer, size, 0);
    if (length == 0) {
      LOG_INFO("socket recvieve exit\n");
      break;
    } else if (length < 0 && errno == EAGAIN) {
      //LOG_INFO("socket recvieve failed\n");
      continue;
    } else if (length < 0) {
      break;
    }
    LOG_INFO("socket recvieve length: %d\n", length);

    if (callback_) {
      callback_(cilent_socket, buffer, length);
    }
  }
  close(cilent_socket);
  return 0;
}

void TCPServer::Accepted() {
  LOG_INFO("TCPServer::Accepted\n");
  struct timeval interval = {3, 0};
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(struct timeval));
  while (!quit_) {
    int cilent_socket;
    socklen_t sosize = sizeof(clientAddress);
    cilent_socket = accept(sockfd, (struct sockaddr*)&clientAddress, &sosize);
    if (cilent_socket < 0) {
      if (errno != EAGAIN) {
        LOG_ERROR("Error socket accept failed %d %d\n", cilent_socket, errno);
      }
      continue;
    }
    LOG_DEBUG("socket accept ip %s\n", inet_ntoa(clientAddress.sin_addr));

    std::shared_ptr<std::thread> recv_thread;
    recv_thread = make_shared<thread>(&TCPServer::Recvieve, this, cilent_socket);
    recv_thread->detach();
    // recv_thread->join();
    // recv_thread = nullptr;
    // close(cilent_socket);
    LOG_DEBUG("socket accept close\n");
  }
  LOG_INFO("socket accept exit\n");
}

int TCPServer::Process(int port) {
  LOG_INFO("TCPServer::Process\n");
  int opt = 1;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    LOG_ERROR("Failed to create socket with tunner");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddress, 0, sizeof(serverAddress));
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    LOG_ERROR("Error setsockopt\n");
    exit(EXIT_FAILURE);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(port);
  if ((::bind(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) < 0) {
    LOG_ERROR("Error bind\n");
    exit(EXIT_FAILURE);
  }
  if (listen(sockfd, 5) < 0) {
    LOG_ERROR("Error listen\n");
    exit(EXIT_FAILURE);
  }

  std::unique_ptr<std::thread> accept_thread;
  accept_thread = std::unique_ptr<std::thread>(new std::thread(&TCPServer::Accepted, this));
  accept_thread->join();
  accept_thread = nullptr;

  return 0;
}
