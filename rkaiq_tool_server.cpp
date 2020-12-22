#include "camera_infohw.h"
#include "rkaiq_manager.h"
#include "rkaiq_protocol.h"
#include "tcp_server.h"
#include "domain_tcp_client.h"
#include <csignal>
#include <ctime>

#define SERVER_PORT 5543
#define UNIX_DOMAIN "/tmp/UNIX.domain"

DomainTCPClient g_tcpClient;
int quit = 0;
int g_app_run_mode = APP_RUN_STATUS_TUNRING;
int g_width = 1920;
int g_height = 1080;
int g_device_id = 0;
int g_rtsp_en = 0;

std::string iqfile;
std::string g_sensor_name;
std::shared_ptr<TCPServer> tcpServer;
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
std::shared_ptr<RKAiqMedia> rkaiq_media;

void sigterm_handler(int sig) {
    fprintf(stderr, "sigterm_handler signal %d\n", sig);
    quit = 1;
    tcpServer->SaveExit();
}

static int get_env(const char* name, int* value, int default_value) {
    char* ptr = getenv(name);
    if(NULL == ptr) {
        *value = default_value;
    } else {
        char* endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        errno = 0;
        *value = strtoul(ptr, &endptr, base);
        if(errno || (ptr == endptr)) {
            errno = 0;
            *value = default_value;
        }
    }
    return 0;
}

static const char short_options[] = "i:m:Dd:w:h:r:";
static const struct option long_options[] = {
    {"iqfile", required_argument, NULL, 'i'},
    {"mode", required_argument, NULL, 'm'},
    {"width", no_argument, NULL, 'w'},
    {"height", no_argument, NULL, 'h'},
    {"device_id", required_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};

static void parse_args(int argc, char** argv) {
    for(;;) {
        int idx;
        int c;
        c = getopt_long(argc, argv, short_options, long_options, &idx);
        if(-1 == c) {
            break;
        }
        switch(c) {
            case 0:
                break;
            case 'i':
                iqfile = optarg;
                break;
            case 'm':
                g_app_run_mode = atoi(optarg);
                break;
            case 'w':
                g_width = atoi(optarg);
                break;
            case 'h':
                g_height = atoi(optarg);
                break;
            case 'd':
                g_device_id = atoi(optarg);
                break;
            default:
                break;
        }
    }
    if(iqfile.empty()) {
#ifdef ANDROID
        iqfile = "/vendor/etc/camera/rkisp1";
#else
        iqfile = "/oem/etc/iqfiles";
#endif
    }
}

int main(int argc, char** argv) {
    LOG_ERROR("#### AIQ tool server 20201222-0933 ####\n");

    signal(SIGQUIT, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGXCPU, sigterm_handler);
    signal(SIGIO, sigterm_handler);
    signal(SIGPIPE, sigterm_handler);

    get_env("rkaiq_tool_server_log_level", &log_level, 0);

    parse_args(argc, argv);
    LOG_DEBUG("iqfile cmd_parser.get  %s\n", iqfile.c_str());
    LOG_DEBUG("g_mode cmd_parser.get  %d\n", g_app_run_mode);
    LOG_DEBUG("g_width cmd_parser.get  %d\n", g_width);
    LOG_DEBUG("g_height cmd_parser.get  %d\n", g_height);
    LOG_DEBUG("g_device_id cmd_parser.get  %d\n", g_device_id);

    rkaiq_manager = std::make_shared<RKAiqToolManager>();

    if(g_tcpClient.Setup("/tmp/UNIX.domain") == false) {
        LOG_INFO("domain connect failed\n");
        return -1;
    }
    //g_tcpClient.Send("UNIX.domain connect success,this is test data", 45);
    LOG_INFO("domain connect success\n");

    tcpServer = std::make_shared<TCPServer>();
    tcpServer->RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
    tcpServer->Process(SERVER_PORT);
    while(!quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    fprintf(stderr, "go quit %d\n", quit);
    tcpServer->SaveExit();

    rkaiq_manager.reset();
    rkaiq_manager = nullptr;
    return 0;
}
