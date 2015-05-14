//
// Created by lk on 2015/4/22.
//

#ifndef BUBI_NET_H
#define BUBI_NET_H

#include <deque>
#include <mutex>
#include <list>
#include <assert.h>
#include <cstring>
#include <typeinfo>
#include <algorithm>
#include <fstream>
#include <json/json.h>

#include "proto/bubi.pb.h"
#include "netbase.h"

class BNode;
extern std::vector<BNode *>vNodes;
extern std::vector<in_addr> hostAddr;
bool AcceptSocket();
BNode* ConnectNode(BService);
void SocketSendData(BNode* bNode);
void SocketRecvData(BNode* bNode);
void ThreadSocketHandler();
void ThreadOpenConnections();
void ThreadMessageHandler();
//get host ip
void Discover();
void time_out(time_t t);
void node_out(BNode *bnode);

class BNetMessageHeader {
public:

    BNetMessageHeader(const std::string& command, unsigned int msize);
    BNetMessageHeader(){}
    unsigned int size() { return (8+bchCommand_.size());}
    std::string serializer();
    void unserializer(std::string str);

    unsigned int nChecksum_;
    unsigned int nMessageSize_;
    std::string bchCommand_;
};

class BNetMessage {
public:
    BNetMessage(const std::string& command, unsigned int msize, const std::string& data) : header_(command, msize), data_(data){}
    BNetMessage(){}
    unsigned int size() { return (data_.size() + header_.size());}
    std::string serializer();
    void unserializer(std::string str);
    BNetMessageHeader header_;
    std::string data_;
};

//information about a peer
class BNode {
public:
    //socket
    SOCKET bSocket_;

    //the massage to send
    std::deque<std::string> vSendMsg_;
    std::mutex mu_vSendMsg_;
    size_t nSendOffset_;
    size_t nSendSize_;
    uint64_t nSendBytes_;

    //the message already received from this peer
    std::deque<BNetMessage> vRecvMsg_;
    std::mutex mu_vRecvMsg_;
    uint64_t nRecvBytes_;

    //this peer ip and port
    BService bService_;
    //time
    time_t nTimeConnected_;
    time_t nLastSend_;
    time_t nLastRecv_;

    //current state about the peer
    bool fDisconnect_;
    //if true, this peer connect the node(which run the bubi core)
    //if false, this node(which run the bubi core) connect the this peer
    bool fInbound_;

    bool fNetworkNode_;

    int nRefCount_;

    BNode(SOCKET bSocket, BService &bService, bool fInbound = false);
    ~BNode();

    bool ReceiveMsgBytes(const char* bch, unsigned int nBytes);
    void PushMessage(const std::string command, const std::string data);
    void CloseSocketDisconnect();
    unsigned int getTotalRecvSize() {
        unsigned int total = 0;
        for (auto &msg : vRecvMsg_)
            total += msg.size();
        return total;
    }
};

#endif //BUBI_NET_H
