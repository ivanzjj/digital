//
// Created by lk on 2015/4/22.
//

#include <mutex>
#include "netbase.h"

//global variable
std::vector<SOCKET> bgListenSocket;

int nConnectTimeout = 5000;

BService::BService(const char* bip, unsigned short bport) {
    port_ = htons(bport);
    inet_pton(AF_INET, bip, &ip_);
}

BService::BService(struct in_addr bip, in_port_t bport) {
    port_ = bport;
    ip_ = bip;
}



BService::BService() {
    port_ = 0;
}

bool operator==(const in_addr &ip1, const in_addr &ip2) {
    if (ip1.s_addr == ip2.s_addr)
        return true;
    return false;
}

bool operator==(const BService &bServiceconst1, const BService &bServiceconst2) {
    if (bServiceconst1.getPort() == bServiceconst2.getPort() && bServiceconst1.getIp() == bServiceconst2.getIp())
        return true;
    return false;
}

bool SetSocketNonBlocking(SOCKET &bSocket, bool fNonBlocking) {

    if (fNonBlocking) {
        int flag = fcntl(bSocket, F_GETFL, 0);
        if (fcntl(bSocket, F_SETFL, flag | O_NONBLOCK) == SOCKET_ERROR) {
            CloseSocket(bSocket);
            return false;
        }
    }
    else {
        int flag = fcntl(bSocket, F_GETFL, 0);
        if (fcntl(bSocket, F_SETFL, flag & ~O_NONBLOCK) == SOCKET_ERROR) {
            CloseSocket(bSocket);
            return false;
        }
    }
    return true;
}

bool CloseSocket(SOCKET &bSocket) {
    if (bSocket == INVALID_SOCKET)
        return false;
    int ret = close(bSocket);
    bSocket = INVALID_SOCKET;
    return ret != SOCKET_ERROR;
}

bool ConnectSocket(const BService &bService, SOCKET& bSocketRet, int btimeout) {
    bSocketRet = INVALID_SOCKET;
    struct sockaddr_in sockaddrIn;
    socklen_t len = sizeof(sockaddrIn);
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr = bService.getIp();
    sockaddrIn.sin_port = bService.getPort();
    SOCKET bSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (bSocket == INVALID_SOCKET)
        return false;
    if (!SetSocketNonBlocking(bSocket, true))
        std::cout << "function ConnectSocket Error : SetSocketNonBlocking failed" << std::endl;
    if (connect(bSocket, (struct sockaddr*)&sockaddrIn, len) == SOCKET_ERROR) {
        if (errno == EINPROGRESS) {
            struct timeval timeout = MillisToTimeval(btimeout);
            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET(bSocket, &fdSet);
            int nRet = select(bSocket+1, NULL, &fdSet, NULL, &timeout);
            if (nRet == 0) {
                std::cout << "connect to" << inet_ntoa(bService.getIp()) << "timeout" << std::endl;
                CloseSocket(bSocket);
                return false;
            }
            if (nRet == SOCKET_ERROR) {
                std::cout << "function ConnectSocket Error : select() failed" << std::endl;
                CloseSocket(bSocket);
                return false;
            }
            //std::cout << "(1)Ret:" << nRet << std::endl;
            socklen_t nRetSize = sizeof(nRet);
            if (getsockopt(bSocket, SOL_SOCKET, SO_ERROR,&nRet, &nRetSize) == SOCKET_ERROR) {
                std::cout << "function ConnectSocket Error : getsockopt error" << std::endl;
                CloseSocket(bSocket);
                return false;
            }
            //std::cout << "(2)Ret:" << nRet << std::endl;
            if (nRet != 0) {
                std::cout << "connect to " << inet_ntoa(bService.getIp()) << "failed after select" << std::endl;
                CloseSocket(bSocket);
                return false;
            }
        }
        else {
            std::cout << "connect to " << inet_ntoa(bService.getIp()) << "failed" << std::endl;
            CloseSocket(bSocket);
            return false;
        }
    }
    bSocketRet = bSocket;
    return true;
}

bool BindListenPort(const BService &bService){
    struct sockaddr_in sockaddrIn;
    socklen_t len = sizeof(sockaddrIn);
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = bService.getPort();
    sockaddrIn.sin_addr = bService.getIp();
    SOCKET bListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (bListenSocket == INVALID_SOCKET) {
        std::cout << "can not open socket" << std::endl;
        return false;
    }
    int nr = 1;
    setsockopt(bListenSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nr, sizeof(int));
    if (!SetSocketNonBlocking(bListenSocket, true)) {
        std::cout << "function BindListenPort Error : set listen socket failed" << std::endl;
        return false;
    }
    //bind
    if (bind(bListenSocket, (struct sockaddr*)&sockaddrIn, len) == SOCKET_ERROR) {
        if (errno == EADDRINUSE)
            std::cout << "can not bind to " << inet_ntoa(bService.getIp()) << " because addr already in use" << std::endl;
        else
            std::cout << "function BindListenPort Error : bind error" << std::endl;
        CloseSocket(bListenSocket);
        return false;
    }
    std::cout << "bound to " << inet_ntoa(bService.getIp()) << std::endl;
    //listen
    if (listen(bListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "function BindListenPort Error : listen failed" << std::endl;
        CloseSocket(bListenSocket);
        return false;
    }
    bgListenSocket.push_back(bListenSocket);
    return true;
}

struct timeval MillisToTimeval(int nTimeout) {
    struct timeval timeout;
    timeout.tv_sec = nTimeout/1000;
    timeout.tv_usec = (nTimeout%1000) * 1000;
    return timeout;
}
