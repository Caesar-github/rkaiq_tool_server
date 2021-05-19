#include "rkaiq_protocol.h"
#include "tcp_client.h"
#include "domain_tcp_client.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_protocol.cpp"

extern int g_app_run_mode;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern int g_device_id;
extern DomainTCPClient g_tcpClient;
extern std::string iqfile;
extern std::string g_sensor_name;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;

bool RKAiqProtocol::is_recv_running = false;
std::shared_ptr<std::thread> RKAiqProtocol::forward_thread = nullptr;

static int ProcessExists(const char* process_name) {
    FILE* fp;
    char cmd[1024] = {0};
    char buf[1024] = {0};
    snprintf(cmd, sizeof(cmd), "ps -ef | grep %s | grep -v grep", process_name);
    fp = popen(cmd, "r");
    if(!fp) {
        LOG_INFO("popen ps | grep %s fail\n", process_name);
        return -1;
    }
    while(fgets(buf, sizeof(buf), fp)) {
        LOG_INFO("ProcessExists %s\n", buf);
        if(strstr(buf, process_name)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int StopProcess(const char* process, const char* str) {
    int count = 0;
    while(ProcessExists(process) > 0) {
        LOG_INFO("StopProcess %s... \n", process);
        system(str);
        sleep(1);
        count++;
        if(count > 3) {
            return -1;
        }
    }
    return 0;
}

int WaitProcessExit(const char* process, int sec) {
    int count = 0;
    LOG_INFO("WaitProcessExit %s... \n", process);
    while(ProcessExists(process) > 0) {
        LOG_INFO("WaitProcessExit %s... \n", process);
        sleep(1);
        count++;
        if(count > sec) {
            return -1;
        }
    }
    return 0;
}

int RKAiqProtocol::DoChangeAppMode(appRunStatus mode) {
    if(g_app_run_mode == mode) {
        return 0;
    }
    if(mode == APP_RUN_STATUS_CAPTURE) {
        LOG_INFO("Switch to APP_RUN_STATUS_CAPTURE\n");
#ifndef ANDROID
        if(g_rtsp_en) {
            deinit_rtsp();
        }
#endif
        rkaiq_manager.reset();
        rkaiq_manager = nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        rkaiq_media->LinkToIsp(false);
    } else {
        LOG_INFO("Switch to APP_RUN_STATUS_TUNRING\n");
        rkaiq_media->LinkToIsp(true);
        rkaiq_manager = std::make_shared<RKAiqToolManager>();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#ifndef ANDROID
        if(g_rtsp_en) {
            media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
            init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height);
        }
#endif
    }
    g_app_run_mode = mode;
    return 0;
}

static void InitCommandPingAns(CommandData_t* cmd, int ret_status) {
    strncpy((char*)cmd->RKID, RKID_CHECK, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_STATUS;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for(int i = 0; i < cmd->datLen; i++) {
        cmd->checkSum += cmd->dat[i];
    }
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id,
                     int ret_status) {
    char send_data[MAXPACKETSIZE];
    LOG_INFO("enter\n");

    strncpy((char*)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for(int i = 0; i < cmd->datLen; i++) {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_INFO("exit\n");
}

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char* buffer, int size) {
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];

    LOG_INFO("HandlerRawCapMessage:\n");

    // for (int i = 0; i < common_cmd->datLen; i++) {
    //   LOG_INFO("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
    // }

    if(strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
        LOG_INFO("RKID: %s\n", common_cmd->RKID);
    } else {
        LOG_INFO("RKID: Unknow\n");
        return;
    }

    LOG_INFO("cmdID: %d\n", common_cmd->cmdID);

    switch(common_cmd->cmdID) {
        case CMD_ID_CAPTURE_STATUS:
            LOG_INFO("CmdID CMD_ID_CAPTURE_STATUS in\n");
            if(common_cmd->dat[0] == KNOCK_KNOCK) {
                InitCommandPingAns(&send_cmd, READY);
                LOG_INFO("Device is READY\n");
            } else {
                LOG_ERROR("Unknow CMD_ID_CAPTURE_STATUS message\n");
            }
            memcpy(send_data, &send_cmd, sizeof(CommandData_t));
            send(sockfd, send_data, sizeof(CommandData_t), 0);
            LOG_INFO("cmdID CMD_ID_CAPTURE_STATUS out\n\n");
            break;
        case CMD_ID_GET_STATUS:
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
            break;
        default:
            break;
    }
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char* buffer, int size) {
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    LOG_INFO("HandlerTCPMessage:\n");
    LOG_INFO("HandlerTCPMessage CommandData_t: 0x%x\n", sizeof(CommandData_t));
    LOG_INFO("HandlerTCPMessage RKID: %s\n", (char*)common_cmd->RKID);

    // TODO Check APP Mode

    if(strcmp((char*)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
        DoChangeAppMode(APP_RUN_STATUS_CAPTURE);
        RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
    } else if(strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
        DoChangeAppMode(APP_RUN_STATUS_TUNRING);
#ifndef ANDROID
        RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
#endif
    } else if(strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
        HandlerCheckDevice(sockfd, buffer, size);
    } else {
      MessageForward(sockfd, buffer, size);
    }
}

int RKAiqProtocol::doMessageForward(int sockfd)
{
  is_recv_running = true;
  while (is_recv_running) {
    char recv_buffer[MAXPACKETSIZE] = {0};
    int recv_len = g_tcpClient.Receive(recv_buffer, MAXPACKETSIZE);
    if (recv_len > 0) {
      send(sockfd, recv_buffer, recv_len, 0);
    }
  }

  return 0;
}

int RKAiqProtocol::MessageForward(int sockfd, char* buffer, int size)
{
  LOG_INFO("[%s]got data:%d!\n", __func__, size);
  g_tcpClient.Send((char*)buffer, size);

  if (is_recv_running) {
    return 0;
  }

  forward_thread = make_shared<thread>(&RKAiqProtocol::doMessageForward, sockfd);

  return 0;
}

