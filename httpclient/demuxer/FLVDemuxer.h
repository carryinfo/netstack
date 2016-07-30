//
//  FLVDemuxer.h
//  httpclient
//
//  Created by wang xiaoming on 7/24/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef FLVDemuxer_h
#define FLVDemuxer_h

#include "OSAL.h"
#include "IDemuxer.h"
#include "HTTPProtocol.h"
class FLVDemuxer : public IDemuxer, public HTTPProtocolCallback{
public:
    FLVDemuxer(): IDemuxer("FLVDemuxer"), parse_state_(STATE_FLV_HEADER), buffered_len_(0),flv_header_size_(0){
        buffered_data_ = new char[1024*1024];   //  1M
    }
    virtual ~FLVDemuxer(){
        if(buffered_data_){
            delete []buffered_data_;
            buffered_data_ = NULL;
            buffered_len_ = 0;
        }
    }
    virtual int open(const char* url, void* option);
    virtual int close();
    
    virtual int onHeader(int code, const std::map<string, string>& header);
    virtual int onBody(const char* data, int len);
    virtual int onInfo(int code);
    
private:
/*
 * @return
 * -1: 还没完成目标
 * >=0: 完成目标, 返回新消耗data里面的字节数
 */
    int recv_data(uint32_t size, const char* data, int len);
    int parse_flv_tag();
private:
    
    enum PARSE_STATE{
        STATE_FLV_HEADER,
        STATE_FLV_REALHEADER,
        STATE_FLV_TAG_HEADER,
        STATE_FLV_TAG_DATA,
        STATE_FLV_TAG_SIZE,
        STATE_UNKNOWN,
    };
    
    shared_ptr<HTTPProtocol> protocol_;
    PARSE_STATE parse_state_;
    char* buffered_data_;
    int buffered_len_;
    
    int flv_header_size_;
    int cur_tag_type_;
    int cur_tag_timestamp_;
    uint32_t cur_tag_data_size_;
};

#endif /* FLVDemuxer_h */
