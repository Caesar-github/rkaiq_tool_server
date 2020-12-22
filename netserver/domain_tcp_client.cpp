#include "domain_tcp_client.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "domain_tcp_client.cpp"

DomainTCPClient::DomainTCPClient() {
    sock = -1;
}

DomainTCPClient::~DomainTCPClient() {
    if(sock > 0) {
        close(sock);
    }
}

bool DomainTCPClient::Setup(string domainPath) {
    if(sock == -1) {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if(sock == -1) {
            LOG_ERROR("Could not create domain socket\n");
            return false;
        }
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, domainPath.c_str());
    struct timeval timeout = {1, 0}; //1 sec
    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        LOG_ERROR("connect domain server failed. Error");
        return false;
    }
    return true;
}

bool DomainTCPClient::Send(string data) {
    if(sock != -1) {
        if(send(sock, data.c_str(), strlen(data.c_str()), 0) < 0) {
            LOG_ERROR("Send failed : %s\n", data.c_str());
            return false;
        }
    } else {
        return false;
    }
    return true;
}

int DomainTCPClient::Send(char* buff, int size) {
    int ret = -1;
    if(sock != -1) {
        ret = send(sock, buff, size, 0);
        if(ret <= 0) {
            LOG_ERROR("Send buff size %d failed\n", size);
            return ret;
        }
    }
    return ret;
}

string DomainTCPClient::Receive(int size) {
    char buffer[size];
    memset(&buffer[0], 0, sizeof(buffer));
    string reply;
    if(recv(sock, buffer, size, 0) < 0) {
        LOG_ERROR("domain receive failed 1!\n", size);
        return "\0";
    }
    buffer[size - 1] = '\0';
    reply = buffer;
    return reply;
}

int DomainTCPClient::Receive(char* buff, int size) {
    int ret = -1;
    memset(buff, 0, size);
    ret = recv(sock, buff, size, 0);
    if(ret < 0) {
        LOG_ERROR("domain receive failed 2!\n", size);
        return -1;
    }
    return ret;
}
