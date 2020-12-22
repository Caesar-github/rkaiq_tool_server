#ifndef DOMAIN_TCP_CLIENT_H
#define DOMAIN_TCP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "logger/log.h"
using namespace std;

class DomainTCPClient {
    private:
        int sock;
        std::string address;
        int port;
        struct sockaddr_un server;
    public:
        DomainTCPClient();
        virtual ~DomainTCPClient();
        bool Setup(string domainPath);
        bool Send(string data);
        int Send(char* buff, int size);
        string Receive(int size);
        int Receive(char* buff, int size);
};

#endif
