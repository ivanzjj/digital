//
// Created by lk on 2015/4/22.
//

#ifndef BUBI_NETBASE_H
#define BUBI_NETBASE_H

#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>

typedef u_int SOCKET;

#define SOCKET_ERROR -1
#define INVALID_SOCKET (SOCKET)(~0)
//the max number this node can accept peers
#define MAXINBOUND 10

#define MAXRECVSIZE (5*1024*1024)

extern int nConnectTimeout;
extern std::vector<SOCKET> bgListenSocket;
//A combination of a IP address and a port
class BService {
public:
    BService(const char* bip, unsigned short bport);
    BService();
    BService(struct in_addr bip, in_port_t bport);
    unsigned short getPort() const {
        return port_;
    }
    in_addr const &getIp() const {
        return ip_;
    }
    friend bool operator==(const BService &bServiceconst1, const BService &bServiceconst2);

private:
    unsigned short port_;
    struct in_addr ip_;
};

//Disable or enable blocking-mode for a socket
bool SetSocketNonBlocking(SOCKET& bSocket, bool fNonBlocking);

bool ConnectSocket(const BService &bService, SOCKET& bSocketRet, int btimeout);

bool BindListenPort(const BService &bService);

bool CloseSocket(SOCKET &bSocket);

struct timeval MillisToTimeval(int nTimeout);

#endif //BUBI_NETBASE_H
