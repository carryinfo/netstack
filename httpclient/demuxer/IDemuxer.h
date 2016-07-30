//
//  IDemuxer.h
//  httpclient
//
//  Created by wang xiaoming on 7/24/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef IDemuxer_h
#define IDemuxer_h

#include <stdio.h>

class DemuxerCallback{
public:
    enum{
        DATA_TYPE_AUDIO,
        DATA_TYPE_VIDEO,
        DATA_TYPE_AUDIO_CSD,
        DATA_TYPE_VIDEO_CSD,
    };
    virtual int on_data(int type, const char* data, int len, int64_t pts, int64_t dts) = 0;
    virtual int on_info(int code) = 0;
};

class IDemuxer{
public:
    IDemuxer(const char* name) : cb_(NULL), name_(name){}
    void setCallback(DemuxerCallback* cb){cb_ = cb;}
    virtual int open(const char* url, void* option) = 0;
    virtual int close() = 0;
protected:
    DemuxerCallback* cb_;
    const char* name_;
};

#endif /* IDemuxer_h */
