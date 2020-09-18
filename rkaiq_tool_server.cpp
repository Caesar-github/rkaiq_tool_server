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
int g_mode = 0;
int g_dump = 0;
int g_device_id = 0;
std::string iqfile;
std::string g_sensor_name;
std::shared_ptr<TCPServer> tcp;
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
std::shared_ptr<RKAiqMedia> rkaiq_media;

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
  tcp->SaveEixt();
}

static void parse_args(int argc, char **argv);

int main(int argc, char **argv) {
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGIO, sigterm_handler);
  signal(SIGPIPE, sigterm_handler);
  get_env("rkaiq_tool_server_log_level", &log_level, 4);

  parse_args(argc, argv);
  LOG_ERROR("iqfile cmd_parser.get  %s\n", iqfile.c_str());
  LOG_ERROR("g_mode cmd_parser.get  %d\n", g_mode);
  LOG_ERROR("g_dump cmd_parser.get  %d\n", g_dump);
  LOG_ERROR("g_device_id cmd_parser.get  %d\n", g_device_id);

  std::string exe_name = argv[0];
  system(STOP_RKLUNCH_CMD);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  WaitProcessExit("mediaserver", 10);
  WaitProcessExit("ispserver", 10);

  rkaiq_media = std::make_shared<RKAiqMedia>();
  rkaiq_media->GetMediaInfo();
  if (g_dump)
    rkaiq_media->DumpMediaInfo();
  g_sensor_name = rkaiq_media->GetSensorName(g_device_id);
  rkaiq_media->LinkToIsp(true);

  if (app_run_mode == APP_RUN_STATUS_TUNRING) {
    LOG_INFO("app_run_mode %d  [0: turning 1: capture]\n", app_run_mode);
    rkaiq_manager = std::make_shared<RKAiqToolManager>(iqfile, g_sensor_name);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    init_rtsp(g_width, g_height);
  }

  tcp = std::make_shared<TCPServer>();
  tcp->RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
  tcp->Process(SERVER_PORT);
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  fprintf(stderr, "go quit %d\n", quit);
  tcp->SaveEixt();
  if (app_run_mode == APP_RUN_STATUS_TUNRING) {
    deinit_rtsp();
    rkaiq_manager.reset();
    rkaiq_manager = nullptr;
  }
  return 0;
}

static const char short_options[] = "i:m:Dd:";
static const struct option long_options[] = {
    {"iqfile", required_argument, NULL, 'i'},
    {"mode", required_argument, NULL, 'm'},
    {"dump", no_argument, NULL, 'D'},
    {"device_id", required_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}};
static void parse_args(int argc, char **argv) {
  for (;;) {
    int idx;
    int c;
    c = getopt_long(argc, argv, short_options, long_options, &idx);
    if (-1 == c)
      break;
    switch (c) {
    case 0: /* getopt_long() flag */
      break;
    case 'i':
      iqfile = optarg;
      break;
    case 'm':
      g_mode = atoi(optarg);
      break;
    case 'd':
      g_device_id = atoi(optarg);
      break;
    case 'D':
      g_dump = 1;
      break;
    default:
      break;
    }
  }
  if (iqfile.empty()) {
    iqfile = "/oem/etc/iqfiles";
  }
}
