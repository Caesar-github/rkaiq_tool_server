#include "rkaiq_protocol.h"
#include "tcp_server.h"
#include <csignal>
#include <ctime>

#define SERVER_PORT 5543

int quit = 0;
void sigterm_handler(int sig) {
  fprintf(stderr, "sigterm_handler signal %d\n", sig);
  quit = 1;
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGIO, sigterm_handler);
  signal(SIGPIPE, SIG_IGN);

  TCPServer tcp;
  tcp.RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
  tcp.Process(SERVER_PORT);
  while (!quit)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  return 0;
}
