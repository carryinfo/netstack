//
//  main.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/20/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "FLVDemuxer.h"



class MyFlvCallback: public DemuxerCallback{
public:
    virtual int on_data(int type, const char* data, int len, int64_t pts, int64_t dts){
        if(type == DemuxerCallback::DATA_TYPE_AUDIO){
            
        }else if(type == DemuxerCallback::DATA_TYPE_AUDIO_CSD){
            
        }else if(type == DemuxerCallback::DATA_TYPE_VIDEO){
            
        }else if(type == DemuxerCallback::DATA_TYPE_VIDEO_CSD){
            
        }
        
        return 0;
    }
    
    virtual int on_info(int code){
        return 0;
    }
};


int main(int argc, const char * argv[]) {
    
    FLVDemuxer demuxer;
    MyFlvCallback cb;
    demuxer.setCallback(&cb);
//    demuxer.open("http://100.69.199.131:8888/local.flv", NULL);
    demuxer.open("http://121.42.25.75:8080/live/livestream35.flv", NULL);
    
    while(1){
        usleep(2000000);
    }
    return 0;
}
