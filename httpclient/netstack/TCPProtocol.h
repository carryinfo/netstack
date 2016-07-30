//
//  TCPProtocol.h
//  httpclient
//
//  Created by wang xiaoming on 7/20/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef TCPProtocol_h
#define TCPProtocol_h

#include <stdio.h>
#include "IProtocol.h"
#include "Reactor.h"
#include "OSAL.h"

#define WRITE_BLOCK     1024
#define READ_BLOCK      1024

class Reactor;

class TCPProtocol : public IProtocol, public ReactorHandler{
public:
    TCPProtocol():IProtocol("TCPProtocol"), connected_(false){}
    virtual ~TCPProtocol(){}
    
//  IProtocol
    //  xx.xx.xx.xx:port
    virtual int open(const char* url, void* option);
    virtual int close();
    virtual int write(const char* data, int len);
    
//  ReactorHandler
    
    virtual int handle_fds(int mask);
    virtual int get_handler();
    
private:
    int sock_;
    char recv_buf_[READ_BLOCK];
    vector<char> send_buf_;
    Mutex mutex_;
    shared_ptr<Reactor> reactor_;
    bool connected_;
};

#endif /* TCPProtocol_h */
