#include "camera_infohw.h"
#include "rkaiq_manager.h"
#include "rkaiq_protocol.h"
#include "tcp_server.h"
#include <csignal>
#include <ctime>

#define SERVER_PORT 5543

int quit = 0;
bool is_turning_mode = false;
int app_run_mode = APP_RUN_STATUS_TUNRING;
int g_width = 1920;
int g_height = 1080;
std::shared_ptr<RKAiqToolManager> rkaiq_manager;

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
}

int main(int argc, char **argv) {
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGIO, sigterm_handler);
  signal(SIGPIPE, sigterm_handler);
  get_env("rkaiq_tool_server_log_level", &log_level, 4);

  std::string exe_name = argv[0];
  system(STOP_RKLUNCH_CMD);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  WaitProcessExit("mediaserver", 10);
  WaitProcessExit("ispserver", 10);

  if (app_run_mode == APP_RUN_STATUS_TUNRING) {
    LOG_INFO("app_run_mode %d  [0: turning 1: capture]\n", app_run_mode);
#ifdef ENABLE_RSTP_SERVER
    rkaiq_manager = std::make_shared<RKAiqToolManager>();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    init_rtsp(g_width, g_height);
#endif
  }

  TCPServer tcp;
  tcp.RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
  tcp.Process(SERVER_PORT);
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  fprintf(stderr, "go quit %d\n", quit);
  tcp.SaveEixt();
  if (app_run_mode == APP_RUN_STATUS_TUNRING) {
#ifdef ENABLE_RSTP_SERVER
    rkaiq_manager->SaveExit();
    deinit_rtsp();
    rkaiq_manager.reset();
    rkaiq_manager = nullptr;
#endif
  }
  return 0;
}
