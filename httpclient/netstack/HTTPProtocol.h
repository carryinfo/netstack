//
//  HTTPProtocol.h
//  httpclient
//
//  Created by wang xiaoming on 7/22/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef HTTPProtocol_h
#define HTTPProtocol_h

#include "TCPProtocol.h"
#include "url.hpp"

class HTTPProtocolCallback{
public:
    
/*
 *  Response Code
 */
    enum{
        HTTP_CODE_NONE = -1,
        
        HTTP_CODE_OK = 200,
        HTTP_CODE_NOTFOUND = 404,
        HTTP_CODE_BADREQUEST = 400,
        HTTP_CODE_FORBIDDEN = 403,
        HTTP_CODE_INTERNALSERVERERROR = 500,
    };
/*
 *  Response Header's key tag
 */
    static string CONTENT_LENGTH;
    static string CONTENT_RANGE;
    static string TRANSFER_ENCODING;
    static string SERVER;
    static string SET_COOKIES;
    static string CONTENT_TYPE;
    static string CONTENT_ENCODING;
    
    
    virtual int onHeader(int code, const std::map<string, string>& header) = 0;
    virtual int onBody(const char* data, int len) = 0;
    virtual int onInfo(int code) = 0;
};


class HTTPProtocol : public IProtocol, public ProtocolCallback{
public:
    enum ParseState{
        PARSE_STATE_HEDAER,
        PARSE_STATE_BODY,
    };
    HTTPProtocol() : IProtocol("HTTPProtocol"),
        port_(0),parse_state_(PARSE_STATE_HEDAER),cb_(NULL), last_data_(NULL), last_len_(0),
        response_code_(HTTPProtocolCallback::HTTP_CODE_NONE){}
    
    void setCallback(HTTPProtocolCallback* cb){cb_ = cb;}
    
    virtual int open(const char* url, void* option);
    virtual int close();
    virtual int write(const char* data, int len);
    
    virtual int onData(const char* data, int len);
    virtual int onInfo(int code);
    
private:
    int send_request();
    int header_parse_line(const char* data1, int data_len1, const char* data2, int data_len2);
    
private:
    shared_ptr<TCPProtocol> tcp_;
    string host_, path_, query_;
    uint16_t port_;
    url url_;
    ParseState parse_state_;
    HTTPProtocolCallback* cb_;
    map<string,string> header_;
    char* last_data_;
    int last_len_;
    int response_code_;
};

#endif /* HTTPProtocol_h */
