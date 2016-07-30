//
//  TCPProtocol.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/20/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "TCPProtocol.h"
#include <sys/socket.h>
#include <unistd.h>
#include "Reactor.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>

int TCPProtocol::open(const char* url, void* option){
    sock_ = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//  set non-blocking
    int val = fcntl (sock_, F_GETFL, 0);
    if (val == -1)
        return -1;
    val |= O_NONBLOCK;
    if (fcntl (sock_, F_SETFL, val) == -1)
        return -1;
    
    string strUrl = url;
    size_t found = strUrl.find(':');
    if (found == string::npos){
        CILOGE("error format: %s", url);
        return -1;
    }
    
    string ip(strUrl.c_str(), found);
    string port = strUrl.substr(found+1);
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(atoi(port.c_str()));
    uint32_t dwIPAddr = inet_addr(ip.c_str());

    addr.sin_addr.s_addr = dwIPAddr;
    int ret = ::connect(sock_, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    
    if(ret == 0){
        connected_ = true;
        if(cb_){
            cb_->onInfo(ProtocolCallback::TCP::INFO_CONNECT_SUCCESS);
        }
    }else{
        if (errno != ETIME && errno != EWOULDBLOCK
            && errno != EINPROGRESS){
            CILOGE("connect: %s, fail", url);
            connected_ = false;
            if(cb_){
                cb_->onInfo(ProtocolCallback::TCP::INFO_CONNECT_FAIL);
            }
            return -1;
        }
    }
    
    reactor_ = Reactor::getReactor();
    reactor_->init();
    reactor_->register_handler( shared_from_this(), MASK_READ | MASK_WRITE | MASK_EXCEPTION);
    reactor_->start();
    return ret;
}

int TCPProtocol::close(){
    if(sock_ != -1){
        ::close(sock_);
        sock_ = -1;
    }
    return 0;
}

int TCPProtocol::write(const char* data, int len){
    Autolock _LOCK(mutex_);
    for(int i=0; i<len; i++){
        send_buf_.push_back(data[i]);
    }
    reactor_->register_handler(shared_from_this(), MASK_WRITE);
    return 0;
}




/////////////////////////////////////////////
////// internal
/////////////////////////////////////////////

int TCPProtocol::handle_fds(int mask){
    if(mask & MASK_READ){
        int num = ::recv(sock_, recv_buf_, READ_BLOCK, 0);
        if(cb_ && num >0){
            cb_->onData(recv_buf_, num);
        }
    }
    
    if(mask & MASK_WRITE){
        Autolock _LOCK(mutex_);
        if(!connected_){
            if(cb_){
                cb_->onInfo(ProtocolCallback::TCP::INFO_CONNECT_SUCCESS);
            }
            connected_ = true;
        }
        int tosend = (send_buf_.size() > WRITE_BLOCK ? WRITE_BLOCK: send_buf_.size());
        if(tosend > 0){
            int num = ::send(sock_, &send_buf_[0], tosend, 0);
            vector<char>::iterator begin = send_buf_.begin();
            vector<char>::iterator end = begin + num;
            send_buf_.erase(begin, end);
        }
        if(send_buf_.size() <= 0){
            reactor_->remove_handler(shared_from_this(), MASK_WRITE);
        }
    }
    
    if(mask & MASK_EXCEPTION){
        //  TODO
        CILOGE("got MASK_EXCEPTION");
        if(cb_){
            if(connected_){
                cb_->onInfo(ProtocolCallback::TCP::INFO_DISCONNECTED);
            }else{
                cb_->onInfo(ProtocolCallback::TCP::INFO_CONNECT_FAIL);
            }
        }
    }
    
    return 0;
}

int TCPProtocol::get_handler(){
    return sock_;
}