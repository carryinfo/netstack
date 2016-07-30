//
//  HTTPClient.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/20/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "HTTPClient.h"

FILE* file = NULL;

int HTTPClient::open(const char* url, int flags, void* option){
    if(!http_){
        http_.reset(new HTTPProtocol);
    }
    http_->setCallback(this);
    http_->open(url, NULL);
    file = fopen("/Users/wangxiaoming/xx1x.jpg", "wb");
    return 0;
}

int HTTPClient::read(uint8_t* buf, int len){
    
    return 0;
}

int HTTPClient::close(){
    http_->close();
    return 0;
}

int HTTPClient::onHeader(int code, const std::map<string, string>& header){
    CILOGE("onHeader, code: %d", code);
    return 0;
}

int HTTPClient::onBody(const char* data, int len){
    printf("write size: %d\n", len);
    fwrite(data, 1, len, file);
    return 0;
}

int HTTPClient::onInfo(int code){
    switch (code) {
        case ProtocolCallback::TCP::INFO_CONNECT_FAIL:
            CILOGE("fail to connect the server");
            break;
        case ProtocolCallback::TCP::INFO_CONNECT_TIMEOUT:
            CILOGE("timeout to connect the server");
        default:
            break;
    }
    return 0;
}
