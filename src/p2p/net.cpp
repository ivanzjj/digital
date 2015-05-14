//
// Created by lk on 2015/4/22.
//

#include <ifaddrs.h>
#include <sstream>
#include "net.h"
#include "interface.h"
using namespace Bubi;
std::vector<BNode *> vNodes;
std::mutex mu_vNodes;
std::vector<struct in_addr> vAddrToSend;
std::mutex mu_vAddrToSend;
static std::list<BNode *> vNodesDisconnected;
std::vector<struct in_addr> hostAddr;
std::vector<BService *> vAddrToConnect;

BNode::BNode(SOCKET bSocket, BService &bService, bool fInbound) {

    bSocket_        = bSocket;
    bService_       = bService;
    fDisconnect_    = false;
    nRecvBytes_     = 0;
    nSendBytes_     = 0;
    nSendOffset_    = 0;
    nSendSize_      = 0;
    nTimeConnected_ = time(NULL);
    nLastRecv_      = 0;
    nLastSend_      = 0;
    fInbound_       = fInbound;
    fNetworkNode_   = false;
}

BNode::~BNode() {
    CloseSocket(bSocket_);
}

void BNode::CloseSocketDisconnect() {
    fDisconnect_ = true;
    if (bSocket_ != INVALID_SOCKET) {
        std::cout << "disconnect peer" << std::endl;
        CloseSocket(bSocket_);
    }
}

BNetMessageHeader::BNetMessageHeader(const std::string& command, unsigned int msize) {
    nMessageSize_ = msize;
    nChecksum_ = 0;
    bchCommand_ = command;
}

std::string BNetMessageHeader::serializer() {
    protocol::bubiMsgHeader bmh;
    bmh.set_checksum(nChecksum_);
    bmh.set_command(bchCommand_);
    bmh.set_msgsize(nMessageSize_);
    std::string out;
    if (!bmh.SerializeToString(&out))
        std::cout << "class " << typeid(*this).name() << " function " <<__FUNCTION__ << "failed" << std::endl;
    return out;
}

void BNetMessageHeader::unserializer(std::string str) {
    protocol::bubiMsgHeader bmh;
    if (!bmh.ParseFromString(str)) {
        std::cout << "class " << typeid(*this).name() << " function " <<__FUNCTION__ << " failed" << std::endl;
    }
    nChecksum_ = bmh.checksum();
    bchCommand_ = bmh.command();
    nMessageSize_ = bmh.msgsize();
}

std::string BNetMessage::serializer() {
    protocol::bubiMsg bm;
    bm.set_data(data_);
    protocol::bubiMsgHeader *bmh = new protocol::bubiMsgHeader();
    bmh->set_checksum(header_.nChecksum_);
    bmh->set_command(header_.bchCommand_);
    bmh->set_msgsize(header_.nMessageSize_);
    bm.set_allocated_header(bmh);
    std::string out;
    if (!bm.SerializeToString(&out))
        std::cout << "class " << typeid(*this).name() << "function " <<__FUNCTION__ << " failed" << std::endl;
    return out;
}

void BNetMessage::unserializer(std::string str) {
    protocol::bubiMsg bm;
    if (!bm.ParseFromString(str)) {
        std::cout << "class " << typeid(*this).name() << " function " <<__FUNCTION__ << "failed" << std::endl;
    }
    data_ = bm.data();
    protocol::bubiMsgHeader bmh;
    bmh = bm.header();
    header_.nMessageSize_ = bmh.msgsize();
    header_.bchCommand_ = bmh.command();
    header_.nChecksum_ = bmh.checksum();

}

BNode* FindNode(BService &bService) {
    std::lock_guard<std::mutex> lockGuard(mu_vNodes);
    for (auto &i : vNodes)
        if (i->bService_ == bService)
            return i;
    return nullptr;
}

BNode* ConnectNode(BService bService) {
    //Look for an existing connection
    BNode* pnode = FindNode(bService);
    if (pnode) {
        pnode->nRefCount_++;
        return pnode;
    }
    SOCKET bSocket;
    if (ConnectSocket(bService, bSocket, nConnectTimeout)) {
        BNode* bNode = new BNode(bSocket, bService, false);
        bNode->nRefCount_++;
        {
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            vNodes.push_back(bNode);
        }
        return bNode;
    }
    return nullptr;

}

bool AcceptSocket() {
    fd_set fdSetRecv;
    FD_ZERO(&fdSetRecv);
    SOCKET hSocketMax = 0;
    for (auto &listenSocket : bgListenSocket) {
        FD_SET(listenSocket, &fdSetRecv);
        hSocketMax = std::max(hSocketMax, listenSocket);
    }
    struct timeval timeval1 = MillisToTimeval(1000);
    int nRet = select(hSocketMax+1, &fdSetRecv, NULL, NULL, &timeval1);
    if (nRet == SOCKET_ERROR) {
        std::cout << "function AcceptSocket select faild " << std::endl;
        return false;
    }
    if (nRet == 0)
        std::cout << "function " << __FUNCTION__ << " select timeout" << std::endl;
    for (auto &listenSocket : bgListenSocket) {
        if (listenSocket != INVALID_SOCKET && FD_ISSET(listenSocket, &fdSetRecv)) {
            struct sockaddr_in sockaddrIn;
            socklen_t len = sizeof(sockaddrIn);
            SOCKET hSocket = accept(listenSocket, (struct sockaddr*)&sockaddrIn, &len);
            BService bService(sockaddrIn.sin_addr, sockaddrIn.sin_port);
            int nInbound = 0;
            {
                std::lock_guard<std::mutex> lockGuard(mu_vNodes);
                for (auto i : vNodes){
                    if (i->fInbound_)
                        ++nInbound;
                }
            }
            if (hSocket == INVALID_SOCKET) {
                std::cout << "function AcceptSocket accept failed " << std::endl;
                return false;
            }
            else if (nInbound > MAXINBOUND) {
                std::cout << "function " << __FUNCTION__ << " inbound number too much " << std::endl;
                CloseSocket(hSocket);
                return false;
            }
            else {
                BNode *bNode = new BNode(hSocket, bService, true);

                ++bNode->nRefCount_;
                {
                    std::lock_guard<std::mutex> lockGuard(mu_vNodes);
                    vNodes.push_back(bNode);
                }
                return true;
            }
        }
    }
    return false;
}

void time_out(time_t t) {

    char *bu = ctime(&t);
    std::cout << bu << std::endl;
}

void Discover() {
    struct ifaddrs *myaddrs;
    struct sockaddr_in *ss;
    if (getifaddrs(&myaddrs) == 0) {
        for (struct ifaddrs *ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;
            if (strcmp(ifa->ifa_name, "lo") == 0)
                continue;
            ss = (struct sockaddr_in *)ifa->ifa_addr;
            if (ss->sin_family == AF_INET)
                hostAddr.push_back(ss->sin_addr);
        }
    }
}
//todo: getBalance function
/*
bool getBalance(const std::string addr, double &balance) {
    //get balance by account addr
    //test

    balance = 200.10;
    return true;
}
*/
std::string ParseHandleJsonData(const std::string data) {
    Json::Reader reader;
    Json::Value  root;
    Json::Value  output;
    Json::FastWriter writer;
    std::string str;
    if (reader.parse(data, root)) {
        if (root["versionClient"].asString() == "1.0") {
            if (root["methodType"].asString() == "1000") {
                //balance request
                double balance;
                std::string balance_str;
				balance =  get_balance(root["params"]["bubiAddr"].asString());
                output["errCode"] = "1";
                output["msg"] = "success";
                std::stringstream ss;
                ss << balance;
                ss >> balance_str;
                std::cout << "balance_str:" << balance_str << std::endl;
                output["data"]["bubiAddr"] = root["params"]["bubiAddr"];
                output["data"]["balance"] = balance_str;
                str = writer.write(output);
                return str;

            }
            else if (root["methodType"].asString() == "1001") {
                //transaction
                std::string sendAddr = root["params"]["sendAddr"].asString();
                std::string fee = root["params"]["fee"].asString();
                std::string recvAddr = root["params"]["receive_addr"].asString();
                //todo: use variables above to generate transactions
                //todo: put transaction into ledger
				std::stringstream ss;
				ss << fee;
				double amount;
				ss >> amount;
                if (!create_transaction(sendAddr, recvAddr, amount)) {

					output["errCode"] = "1";
					output["msg"] = "success";
				}
				else {
					output["errCode"] = "0";
					output["msg"] = "fail";
				}
				time_t tm = time(NULL);
				std::stringstream kk;
				kk << tm;
				std::string transid;
				kk >> transid;
				output["data"]["trans"] = transid;
                //use protobuf serialize tx to a string
                std::string txserialize;
                //test
                txserialize = "the serialization of this tx";
                {
                    std::lock_guard<std::mutex> lockGuard(mu_vNodes);
                    for (auto node : vNodes) {
                        if (!node->fInbound_)
                            node->PushMessage("transaction", txserialize);
                    }
                }
                str = writer.write(output);
                return str;
            }
            else if (root["methodType"].asString() == "1002") {

            }
            else if (root["methodType"].asString() == "1003") {

            }
            else if (root["methodType"].asString() == "1004") {
                //create user
                std::string bubiAddr = root["params"]["bubiAddr"].asString();
                //todo: create account
				
			    if (!create_account(bubiAddr)) {	
					output["errCode"] = "1";
					output["msg"] = "success";
				}
				else {
					output["errCode"] = "0";
					output["msg"] = "fail";
				}
                output["data"] = Json::Value();
				str = writer.write(output);
				return str;
            }
			else {
				output["errCode"] = "0";
				output["msg"] = "the methodType does not exits";
				str = writer.write(output);
				return str;
			}
        }
        else {
            std::cout << "the client version is not correct" << std::endl;
        }
    }
    else {
        std::cout << "received json data from client is null" << std::endl;
    }
}

bool BNode::ReceiveMsgBytes(const char *bch, unsigned int nBytes) {

}
//require lock mu_vSendmsg
void BNode::PushMessage(const std::string command, const std::string data) {
    BNetMessage message(command, 0, data);
    std::string msg = message.serializer();
    //std::cout << "sending msg:" << msg << std::endl;
    vSendMsg_.push_back(msg);
}
//require lock mu_vSendmsg_
void SocketSendData(BNode* bNode){
    std::cout << "SocketSendData......" << std::endl;
    std::deque<std::string>::iterator it = bNode->vSendMsg_.begin();
    while (it != bNode->vSendMsg_.end()) {
        const std::string &data = *it;
        assert(data.size() > bNode->nSendOffset_);
        int nBytes = send(bNode->bSocket_, &data[bNode->nSendOffset_], data.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
        if (nBytes > 0) {
            std::cout << "send Bytes:" << nBytes << std::endl;
            bNode->nLastSend_   = time(NULL);
            bNode->nSendOffset_ += nBytes;
            bNode->nSendBytes_  += nBytes;
            if (bNode->nSendOffset_ == data.size()) {
                bNode->nSendOffset_ = 0;
                //bNode->nSendSize_ -= data.size();
                ++it;
                //sleep(3);
            }
            else {
                //can not send full message; stop sending more
                break;
            }
        }
        else {
            if (nBytes < 0) {
                if (errno == EINPROGRESS || errno == EMSGSIZE || errno == EINTR || errno == EWOULDBLOCK)
                    std::cout << __FUNCTION__ << "--ERROR: EINPROGRESS / EMSGSIZE / EINTR / EWOULDBLOCK" << std::endl;
                if (errno != EINPROGRESS && errno != EMSGSIZE && errno != EINTR && errno != EWOULDBLOCK) {
                    std::cout << "function SocketSendData Error: socket send error" << std::endl;
                    bNode->CloseSocketDisconnect();
                }
            }
        }
    }
    if (it == bNode->vSendMsg_.end()) {
        assert(bNode->nSendOffset_ == 0);
        //assert(bNode->nSendSize_ == 0);
    }
    //delete msg already send successfully
    std::cout << "delete message from vsendmsg" << std::endl;
    bNode->vSendMsg_.erase(bNode->vSendMsg_.begin(), it);
}
//require lock mu_vRecvmsg
void SocketRecvData(BNode* bNode) {
    std::cout << "SocketRecvData......" << std::endl;
    //typical socket buffer is 8K-64K
    char bchBuf[0x10000];
    //MSG_DONTWAIT--Enables nonblocking operation; if the operation would block, the call fails with the
    //error EAGAIN or EWOULDBLOCK
    int nBytes = recv(bNode->bSocket_, bchBuf, sizeof(bchBuf), MSG_DONTWAIT);
    if (nBytes > 0) {
        std::cout << "nBytes:" << nBytes << std::endl;
        bNode->nLastRecv_ = time(NULL);
        bNode->nRecvBytes_ += nBytes;
        //bchBuf[nBytes] = '\0';
        std::string data(bchBuf,nBytes);
        std::cout << "received data : " << data << std::endl;
        //data is json
        //parse data
        if (data[0] != '{') {
            BNetMessage message;
            message.unserializer(data);
            std::cout << "received msg data:  command: " << message.header_.bchCommand_ << " is:" << message.data_ <<
            std::endl;
            bNode->vRecvMsg_.push_back(message);
        }
        else {
            std::string jsondata = ParseHandleJsonData(data);
            int nBytes = send(bNode->bSocket_, &jsondata[0], jsondata.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            std::cout << "json send bytes:" << nBytes << std::endl;
        }
        /*
        {
            std::cout << "begin to push message to vRecvMsg" << std::endl;
            std::lock_guard<std::mutex> lock_guard(bNode->mu_vRecvMsg_);
            bNode->vRecvMsg_.push_back(message);
            std::cout << "push message successfully" << std::endl;
        }
         */
    }
    else if (nBytes == 0) {
        std::cout << "nBytes == 0" << std::endl;
        bNode->CloseSocketDisconnect();
    }
    else if(nBytes < 0) {
        std::cout << "nBytes < 0" << " SOCKET:" << bNode->bSocket_ << std::endl;
        if (errno != EWOULDBLOCK && errno != EINTR && errno != EMSGSIZE && errno != EINPROGRESS) {
            bNode->CloseSocketDisconnect();
        }
    }
}

//Thread safe function
void SendMessages(const std::string command, const std::string data) {
    std::lock_guard<std::mutex> lockGuard(mu_vNodes);
    for (auto node : vNodes) {
        if (!node->fInbound_) {
            std::lock_guard<std::mutex> lockGuard1(node->mu_vSendMsg_);
            node->PushMessage(command, data);
        }
    }
}

void ThreadOpenConnections() {
    std::cout << "ThreadOpenConnections started......" << std::endl;
    std::ifstream infile("/etc/bubiconfig/peers.dat");
    std::string ip;
    if (!infile)
        std::cout << "peers.dat open failed!" << std::endl;
    while (infile >> ip) {
        BService *bService = new BService(ip.c_str(), 30000);
        vAddrToConnect.push_back(bService);
    }
    infile.close();
    while (true) {
        for (auto bservice : vAddrToConnect) {
            ConnectNode(*bservice);
            sleep(1);
        }
        {
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            int size = 0;
            for (auto node : vNodes) {
                if (!node->fInbound_)
                    ++size;
            }
            if (size == vAddrToConnect.size())
                break;
        }
        /*
        mu_vNodes.lock();
        for (auto node : vNodes) {
            std::cout << "INFO--ip:" << inet_ntoa(node->bService_.getIp()) << " port:"
            << ntohs(node->bService_.getPort()) << " nconnectedtime:" << node->nTimeConnected_
            << " SOCKET:" << node->bSocket_ << " nRef:" << node->nRefCount_ << std::endl;
            node->PushMessage("tx","a give b 100 dollars!");
            std::cout << "size:" << node->vSendMsg_.size() << std::endl;
        }
        mu_vNodes.unlock();*/
    }
    std::cout << "ThreadOpenConnections finished................................." << std::endl;
}

void ProcessMessages(BNode *node, std::vector<BNode *>& vNodeCopy) {
    auto it = node->vRecvMsg_.begin();
    while (it != node->vRecvMsg_.end()) {

        if (it->header_.bchCommand_ == "transaction") {
            std::lock_guard<std::mutex> lockGuard(node->mu_vSendMsg_);
            std::cout << "tx reply......" << std::endl;
            node->PushMessage("tx-reply", "i have got your tx message......");
        }
        ++it;
    }
    node->vRecvMsg_.erase(node->vRecvMsg_.begin(), it);
}

void ThreadMessageHandler() {
    std::cout << "ThreadMessageHandler started......" << std::endl;
    while (true) {
        std::vector<BNode *> vNodeCopy;
        {
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            vNodeCopy = vNodes;
        }
        for (auto bnode : vNodeCopy) {
            if (bnode->mu_vRecvMsg_.try_lock()) {
                //process message
                ProcessMessages(bnode, vNodeCopy);
                bnode->mu_vRecvMsg_.unlock();
            }
            if (bnode->mu_vSendMsg_.try_lock()) {
                //
                /*
                sleep(10);
                if (bnode->fInbound_)
                    bnode->PushMessage("jiaoyi", "ss give kk 100 dollars");
                    */
                bnode->mu_vSendMsg_.unlock();
            }
        }
        sleep(1);
    }
}

void node_out(BNode *bnode) {
    std::cout << "nLastRecvTime:" << bnode->nLastRecv_ << std::endl;
    std::cout << "=======vNodes INFO--ip:" << inet_ntoa(bnode->bService_.getIp()) << " port:" << ntohs(bnode->bService_.getPort())
    << " SOCKET:" << bnode->bSocket_ << " nRef:" << bnode->nRefCount_ << std::endl;
    std::cout << " nConnectedTime:";
    time_out(bnode->nTimeConnected_);
    std::cout << "nLastRecvTime:";
    time_out(bnode->nLastRecv_);
    std::cout << "nLastSendTime:";
    time_out(bnode->nLastSend_);
    std::cout << "\n";
}

void ThreadSocketHandler() {
    std::cout << "ThreadSocketHandler started......" << std::endl;

    while (true) {
        {
            //1.disconnect nodes
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            std::vector<BNode *> vNodesCopy = vNodes;

            for (auto bNode : vNodesCopy) {
                if (bNode->fDisconnect_ && bNode->vSendMsg_.empty() && bNode->vRecvMsg_.empty()) {
                //if (bNode->fDisconnect_ || (bNode->nRefCount_ <= 0 && bNode->vRecvMsg_.empty() && bNode->nSendSize_ == 0 && bNode->vSendMsg_.empty())) {

                    //delete from vNodes
                    vNodes.erase(std::remove(vNodes.begin(), vNodes.end(), bNode), vNodes.end());
                    bNode->CloseSocketDisconnect();
                    delete bNode;
                    //hold in disconnected pool until all refs are released
                    //if (bNode->fInbound_ || bNode->fNetworkNode_)
                    //    bNode->nRefCount_--;
                    //vNodesDisconnected.push_back(bNode);
                }
            }
        }
         /*
        {
            //2. delete disconnected nodes
            std::list<BNode*> vNodeDisconnectedCopy = vNodesDisconnected;
            for (auto bNode : vNodeDisconnectedCopy) {
                if (bNode->nRefCount_ <= 0) {
                    bool fDelete = false;
                    if (bNode->mu_vSendMsg_.try_lock() && bNode->mu_vRecvMsg_.try_lock()) {
                        fDelete = true;
                        bNode->mu_vRecvMsg_.unlock();
                        bNode->mu_vSendMsg_.unlock();
                    }
                    if (fDelete) {
                        vNodesDisconnected.remove(bNode);
                        delete bNode;
                    }
                }
            }
        }*/

        //accept new connections
        //select timeout: 1000ms
        AcceptSocket();
        struct timeval timeout = MillisToTimeval(50);
        fd_set fdsetRecv;
        fd_set fdsetSend;
        FD_ZERO(&fdsetRecv);
        FD_ZERO(&fdsetSend);
        SOCKET maxSocket = 0;
        {
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            std::cout << "1111111111111111111111the size of peers : " << vNodes.size() << std::endl;
            for (auto bnode : vNodes) {
                if (bnode->bSocket_ == INVALID_SOCKET)
                    continue;
                node_out(bnode);
            }
            for (auto bnode : vNodes) {

                if (bnode->bSocket_ == INVALID_SOCKET)
                    continue;
                std::cout << "******************size:" << bnode->vSendMsg_.size() << std::endl;
                if (bnode->mu_vSendMsg_.try_lock()) {
                    if (!bnode->vSendMsg_.empty()) {
                        std::cout << "#######fdsetSend:" << bnode->bSocket_ << std::endl;
                        node_out(bnode);
                        FD_SET(bnode->bSocket_, &fdsetSend);
                        maxSocket = std::max(maxSocket, bnode->bSocket_);
                        bnode->mu_vSendMsg_.unlock();
                        continue;
                    }
                    else
                        bnode->mu_vSendMsg_.unlock();
                }
                if (bnode->mu_vRecvMsg_.try_lock()) {
                    if (bnode->vRecvMsg_.empty() || bnode->getTotalRecvSize() <= MAXRECVSIZE) {
                        FD_SET(bnode->bSocket_, &fdsetRecv);
                        std::cout << "********fdsetRecv:" << bnode->bSocket_ << std::endl;
                        node_out(bnode);
                        maxSocket = std::max(maxSocket, bnode->bSocket_);
                    }
                    bnode->mu_vRecvMsg_.unlock();
                }
            }
        }
        std::cout << "maxSocket:" << maxSocket << std::endl;
        std::cout << "timeout: second:" << timeout.tv_sec << " useconds:" << timeout.tv_usec << std::endl;
        int nSelect = select(maxSocket+1, &fdsetRecv, &fdsetSend, NULL, &timeout);
        if (nSelect == SOCKET_ERROR) {
            std::cout << "function " << __FUNCTION__ << " select failed" << std::endl;
            if (errno == EBADF)
                std::cout << "EBADF" << std::endl;
            if (errno == EINTR)
                std::cout << "EINTR" << std::endl;
            if (errno == EINVAL)
                std::cout << "EINVVAL" << std::endl;
            if (errno == ENOMEM)
                std::cout << "ENOMEM" << std::endl;
            continue;
        }
        if (nSelect == 0) {
            std::cout << "function:" << __FUNCTION__ << " select timeout" << std::endl;
            continue;
        }
        //service each socket
        std::vector<BNode *> vNodeCopy;
        {
            std::lock_guard<std::mutex> lockGuard(mu_vNodes);
            vNodeCopy = vNodes;
        }
        for (auto bnode : vNodeCopy) {
            //receive
            if (bnode->bSocket_ == INVALID_SOCKET)
                continue;
            if (FD_ISSET(bnode->bSocket_, &fdsetRecv)) {

                if (bnode->mu_vRecvMsg_.try_lock()) {
                    std::cout << "-------------------------------------------" << std::endl;
                    std::cout << "******************this socket can read " << bnode->bSocket_ << " ip:" << inet_ntoa(bnode->bService_.getIp())
                    << " port:" << ntohs(bnode->bService_.getPort()) << std::endl;
                    SocketRecvData(bnode);
                    bnode->mu_vRecvMsg_.unlock();
                }
            }
            //send
            else if (FD_ISSET(bnode->bSocket_, &fdsetSend)) {

                if (bnode->mu_vSendMsg_.try_lock()) {
                    std::cout << "-------------------------------------------" << std::endl;
                    std::cout << "############this socket can write " << bnode->bSocket_ << " ip:" << inet_ntoa(bnode->bService_.getIp())
                    << " port:" << ntohs(bnode->bService_.getPort()) << std::endl;
                    SocketSendData(bnode);
                    bnode->mu_vSendMsg_.unlock();
                }
            }
            //inactivity checking
            /*
            int64_t nTime = time(NULL);
            if (nTime - bnode->nTimeConnected_ > 60) {
                if (bnode->nLastRecv_ == 0 || bnode->nLastSend_ == 0) {
                    std::cout << "socket no message in first 60 seconds" << std::endl;
                    bnode->fDisconnect_ = true;
                }
                else if (nTime - bnode->nLastSend_ > 20*60) {
                    std::cout << "socket sending timeout" << std::endl;
                    bnode->fDisconnect_ = true;
                }
            }*/
        }
    }

}
