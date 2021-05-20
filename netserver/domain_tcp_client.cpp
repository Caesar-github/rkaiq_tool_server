#include "domain_tcp_client.h"

#include <errno.h>

#ifdef __ANDROID__
#include <android-base/file.h>
#include <cutils/sockets.h>
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

#ifdef __ANDROID__
#define ANDROID_RESERVED_SOCKET_PREFIX "/dev/socket/"

// Connects to /dev/socket/<name> and returns the associated fd or returns -1 on error.
// O_CLOEXEC is always set.
static int socket_local_client(const std::string& name, int type) {
	sockaddr_un addr = {.sun_family = AF_LOCAL};

	std::string path = "/dev/socket/" + name;
	if (path.size() + 1 > sizeof(addr.sun_path)) {
		return -1;
	}
	strlcpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path));

	int fd = socket(AF_LOCAL, type | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		return -1;
	}

	if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
		close(fd);
		return -1;
	}

	return fd;
}
#endif

DomainTCPClient::DomainTCPClient() {
    sock = -1;
}

DomainTCPClient::~DomainTCPClient() {
    if(sock > 0) {
        close(sock);
    }
}

bool DomainTCPClient::Setup(string domainPath) {
#ifdef __ANDROID__
    sock = socket_local_client(android::base::Basename(domainPath), SOCK_STREAM);
    if (sock < 0) {
        LOG_ERROR("Could not create domain socket %s\n", strerror(errno));
        return false;
    }
#else
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
#endif
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
        LOG_INFO("domain receive failed 2!\n", size);
        return -1;
    }
    return ret;
}