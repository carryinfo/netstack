//
//  IProtocol.h
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef IProtocol_h
#define IProtocol_h

#include <stdint.h>

class ProtocolCallback{
public:
    class TCP{
    public:
        enum{
            //  for TCP
            INFO_CONNECT_SUCCESS,
            INFO_CONNECT_FAIL,
            INFO_CONNECT_TIMEOUT,
            INFO_DISCONNECTED,
        };
    };
    
    virtual int onData(const char* data, int len)=0;
    virtual int onInfo(int code) = 0;
};

class IProtocol{
public:
    IProtocol(const char* name):name_(name), cb_(NULL){}
    virtual ~IProtocol(){}
    virtual int open(const char* url, void* option) = 0;
    virtual int close() = 0;
    virtual int write(const char* data, int len) = 0;
    void setCallback(ProtocolCallback* cb){cb_ = cb;}
protected:
    const char* name_;
    ProtocolCallback* cb_;
};

#endif /* IProtocol_h */
