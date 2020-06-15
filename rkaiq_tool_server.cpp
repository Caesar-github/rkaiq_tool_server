#include "camera_infohw.h"
#include "rkaiq_protocol.h"
#include "tcp_server.h"
#include <csignal>
#include <ctime>

#define SERVER_PORT 5543

int quit = 0;

static int get_env(const char *name, int *value, int default_value) {
  char *ptr = getenv(name);
  if (NULL == ptr) {
    *value = default_value;
  } else {
    char *endptr;
    int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
    errno = 0;
    *value = strtoul(ptr, &endptr, base);
    if (errno || (ptr == endptr)) {
      errno = 0;
      *value = default_value;
    }
  }
  return 0;
}

void sigterm_handler(int sig) {
  fprintf(stderr, "sigterm_handler signal %d\n", sig);
  quit = 1;
  system(STOP_RTSPSERVER_CMD);
  usleep(200000);
  system(STOP_RKLUNCH_CMD);
  usleep(1800000);
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGIO, sigterm_handler);
  signal(SIGPIPE, SIG_IGN);
  get_env("rkaiq_tool_server_log_level", &log_level, 4);

  TCPServer tcp;
  tcp.RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
  tcp.Process(SERVER_PORT);
  while (!quit)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  return 0;
}
