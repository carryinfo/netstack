//
//  HTTPProtocol.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/22/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "HTTPProtocol.h"
#include <netdb.h>
#include "OSAL.h"
#include <string.h>

string HTTPProtocolCallback::CONTENT_LENGTH = "Content-Length";
string HTTPProtocolCallback::CONTENT_RANGE = "Content-Range";
string HTTPProtocolCallback::TRANSFER_ENCODING = "Transfer-Encoding";
string HTTPProtocolCallback::SERVER = "Server";
string HTTPProtocolCallback::SET_COOKIES = "Set-Cookies";
string HTTPProtocolCallback::CONTENT_TYPE = "Content-Type";
string HTTPProtocolCallback::CONTENT_ENCODING = "Content-Encoding";

//////////////////////////////////

int HTTPProtocol::open(const char* url, void* option){
    url_ = url::from_string(url);
    
    if( url_.protocol().compare("http") != 0){
        CILOGE("HTTPProtocol don't support this url: %s", url);
        return -1;
    }
    
    host_ = url_.host();
    path_ = url_.path();
    query_ = url_.query();
    port_ = url_.port();
    
    uint32_t ip = 0;
    hostent* he = gethostbyname(host_.c_str());
    
    if (he){
        ip = *((uint32_t*)(he->h_addr));
    }else{
        CILOGE("gethostbyname fail: %s", url);
        return -1;
    }
    
    char tmp[25]={0};
    sprintf(tmp,"%d.%d.%d.%d:%d", ip & 0xff, (ip & 0xffff) >> 8, (ip & 0xffffff) >> 16, ip >> 24 , port_);
    
    CILOGE("tcp addr: %s", tmp);
    
    
    if(!tcp_){
        tcp_.reset(new TCPProtocol);
    }
    tcp_->setCallback(this);
    return tcp_->open(tmp, NULL);
}

int HTTPProtocol::close(){
    if(tcp_){
        tcp_->close();
    }
    if(last_data_){
        delete []last_data_;
        last_data_ = NULL;
        last_len_ = 0;
    }
    return 0;
}

int HTTPProtocol::write(const char* data, int len){
    if(tcp_){
        tcp_->write(data, len);
    }
    return 0;
}

int HTTPProtocol::onData(const char* data, int len){
    if(len <= 0){
        return  0;
    }
    
    switch (parse_state_) {
        case PARSE_STATE_HEDAER:{
            const char* start = data;
            do{
                char* end = strstr(start, "\r\n");
                if(end != NULL){
                    if(end == start){
                        parse_state_ = PARSE_STATE_BODY;
                        if(cb_){
                            cb_->onHeader(response_code_, header_);
                        }
                        start += 2;
                        return onData(start, len-(start-data));
                    }
                    header_parse_line(last_data_, last_len_, start, end - start);
                    start = end + 2;
                }else{
                    if(last_data_){
                        delete []last_data_;
                        last_data_ = NULL;
                        last_len_ = 0;
                    }
                    last_len_ = len - (start - data);
                    last_data_ = new char[last_len_];
                    memcpy(last_data_, start, last_len_);
                    break;
                }
            }while(start < data + len);
        }
            break;
        case PARSE_STATE_BODY:{
            if(cb_){
                cb_->onBody(data, len);
            }
        }
            break;
        default:
            break;
    }
    return 0;
}

int HTTPProtocol::onInfo(int code){
    switch (code) {
        case ProtocolCallback::TCP::INFO_CONNECT_SUCCESS:
            if(cb_){
                // connect success
            }
            send_request();
            break;
        case ProtocolCallback::TCP::INFO_CONNECT_TIMEOUT:
        case ProtocolCallback::TCP::INFO_CONNECT_FAIL:
            if(cb_){
                cb_->onInfo(ProtocolCallback::TCP::INFO_CONNECT_SUCCESS);
            }
        default:
            break;
    }
    return 0;
}

int HTTPProtocol::send_request(){
    string header;
    char tmp[500]={0};
    header.append("User-Agent: http-agent\r\n");
    header.append("Accept: */*\r\n");
    header.append("Connection: close\r\n");
    sprintf(tmp, "Host: %s\r\n", host_.c_str());
    header.append(tmp);
    
    sprintf(tmp,
             "%s %s?%s HTTP/1.1\r\n"
             "%s"
             "%s"
             "%s"
             "%s"
             "\r\n%s\r\n",
             "GET",
             path_.c_str(),
             query_.c_str(),
             "",
             header.c_str(),
             "",
             "");
    string req(tmp);
    
//    CILOGD("query: %s\n", tmp);
    
    this->write(req.data(), req.size());
    
    return 0;
}

int HTTPProtocol::header_parse_line(const char* data1, int data_len1, const char* data2, int data_len2){
    if(strstr(data2,"HTTP/")){
        const char* p = data2 + 5;
        while(!isspace(*p)) p++;
        response_code_ = strtol(p+1, NULL, 10);
    }
    
    if(strstr(data2, HTTPProtocolCallback::CONTENT_LENGTH.c_str())){
        for(int i=0;i< data_len2; i++){
            printf("%c",data2[i]);
        }
        printf("\n");
    }
    
    //  TODO
    for(int i=0;i< data_len2; i++){
        printf("%c",data2[i]);
    }
    printf("\n");
    return 0;
}
