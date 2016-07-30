//
//  HTTPClient.h
//  httpclient
//
//  Created by wang xiaoming on 7/20/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef HTTPClient_h
#define HTTPClient_h

#include "HTTPProtocol.h"


class HTTPClient : public HTTPProtocolCallback{
public:
    HTTPClient(){}
    int open(const char* url, int flags, void* option);
    int read(uint8_t* buf, int len);
    int close();
protected:
    virtual int onHeader(int code, const std::map<string, string>& header);
    virtual int onBody(const char* data, int len);
    virtual int onInfo(int code);
private:
    shared_ptr<HTTPProtocol> http_;
    vector<uint8_t> vec_queue_;
    Mutex mutex_;
};

#endif /* HTTPClient_h */
