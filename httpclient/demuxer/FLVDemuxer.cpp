//
//  FLVDemuxer.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/24/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "FLVDemuxer.h"

int FLVDemuxer::open(const char* url, void* option){
    if(strncmp(url,"http://", 7) != 0 ){
        CILOGE("error url, not supported by FLVDemuxer: %s", url);
        return -1;
    }
    
    if(!protocol_){
        protocol_.reset(new HTTPProtocol);
        protocol_->setCallback(this);
    }
    
    return protocol_->open(url, option);
}

int FLVDemuxer::close(){
    if(protocol_){
        return protocol_->close();
    }
    return 0;
}

int FLVDemuxer::onHeader(int code, const std::map<string, string>& header){
    return 0;
}

int FLVDemuxer::onBody(const char* data, int len){
    switch (parse_state_) {
        case STATE_UNKNOWN:
        {
            //  TODO 容错处理
            char* p = strnstr(buffered_data_, "FLV", buffered_len_);
            long index = p - buffered_data_;
            for(long i= 0; i< buffered_len_ - index; i++){
                buffered_data_[i] = buffered_data_[index + i];
            }
            buffered_len_ = buffered_len_ - (int)index;
            parse_state_ = STATE_FLV_HEADER;
            return onBody(data, len);
        }
            break;
        case STATE_FLV_HEADER:
        {
            int ret = recv_data(9 , data, len);
            if(ret >= 0){
                if((buffered_data_[0] != 'F') || (buffered_data_[1] != 'L') || (buffered_data_[2] != 'V')){
                    parse_state_ = STATE_UNKNOWN;
                    return onBody(data + ret, len - ret);
                }
                // 大端模式
                unsigned char* _buffer_data = (unsigned char*)buffered_data_;
                flv_header_size_ =    _buffer_data[5]<<24
                                    | _buffer_data[6]<<16
                                    | _buffer_data[7]<<8
                                    | _buffer_data[8];
                CILOGE("flv header size: %d", flv_header_size_);
                if (flv_header_size_ == 9) {
                    parse_state_ = STATE_FLV_TAG_SIZE;
                    buffered_len_ = 0;
                    return onBody(data + ret, len - ret);
                }else{
                    parse_state_ = STATE_FLV_REALHEADER;
                    buffered_len_ = 0;
                    return onBody(data + ret, len - ret);
                }
            }
        }
            break;
        case STATE_FLV_REALHEADER:
        {
            int ret = recv_data(flv_header_size_ - 9, data, len);
            if(ret >= 0 ){
                // TODO extented header
                // ...
                parse_state_ = STATE_FLV_TAG_SIZE;
                buffered_len_ = 0;
                return onBody(data + ret, len - ret);
            }
        }
            break;
        case STATE_FLV_TAG_SIZE:
        {
            int ret = recv_data(4, data, len);
            if(ret >= 0 ){
                unsigned char* _buffer_data = (unsigned char*)buffered_data_;
                int prev_tag_size =   _buffer_data[0] << 24
                                    | _buffer_data[1]<<16
                                    | _buffer_data[2]<<8
                                    | _buffer_data[3];
//                CILOGE("prev tag size: %d", prev_tag_size);
                parse_state_ = STATE_FLV_TAG_HEADER;
                buffered_len_ = 0;
                return onBody(data + ret, len - ret);
            }
        }
            break;
        case STATE_FLV_TAG_HEADER:
        {
            int ret = recv_data(11, data, len);
            if(ret >= 0){
                cur_tag_type_ = buffered_data_[0];
                
                unsigned char* _buffer_data = (unsigned char*)buffered_data_;
                cur_tag_data_size_ = (_buffer_data[1] << 16) | (_buffer_data[2] << 8) | ( _buffer_data[3]);
                cur_tag_timestamp_ =  _buffer_data[7]<<24 | _buffer_data[4]<<16
                                    | _buffer_data[5] << 8 | _buffer_data[6];
                CILOGD("tag type: %d, tag size: %d", cur_tag_type_, cur_tag_data_size_);
                parse_state_ = STATE_FLV_TAG_DATA;
                buffered_len_ = 0;
                return onBody(data + ret, len - ret);
            }
        }
            break;
        case STATE_FLV_TAG_DATA:
        {
            int ret = recv_data(cur_tag_data_size_, data, len);
            if(ret >=0 ){
                parse_flv_tag();
                parse_state_ = STATE_FLV_TAG_SIZE;
                buffered_len_ = 0;
                return onBody(data + ret, len - ret);
            }
        }
            break;
        default:
            break;
    }
    return 0;
}

int FLVDemuxer::onInfo(int code){
    return 0;
}

int FLVDemuxer::recv_data(uint32_t size, const char* data, int len){
    if(buffered_len_ + len < size){
        memcpy(buffered_data_ + buffered_len_, data, len);
        buffered_len_ += len;
        return -1;
    }else{
        int left_size = size - buffered_len_;
        memcpy(buffered_data_ + buffered_len_, data, left_size);
        buffered_len_ = size;
        return left_size;
    }
    return 0;
}

int FLVDemuxer::parse_flv_tag(){
    int index = 0;
    if(cur_tag_type_ == 8){
        //  audio tag
        
        char header = buffered_data_[0];
        
        int sound_format = (header & 0xf0) >> 4;
        int sound_rate = (header & 0x0c) >> 2;
        int sound_size = (header & 0x02) >> 1;
        int sound_type = header & 0x01;
        index += 1;
        
        if(sound_format == 10){
            //  aac
            uint8_t aac_packet_type = buffered_data_[index];
            index++;
            if(aac_packet_type == 0){
                //  AudioSpecificConfig
                CILOGD("got AudioSpecificConfig, len: %d", buffered_len_ - index);
                if(cb_){
                    cb_->on_data(DemuxerCallback::DATA_TYPE_AUDIO_CSD, buffered_data_ + index, buffered_len_ - index, 0 ,0);
                }
            }else if(aac_packet_type == 1){
                //  Raw AAC frame data
                int64_t pts = cur_tag_timestamp_;
                CILOGD("got Raw AAC frame, sound_format:%d, sound_rate: %d, sound_size:%d, soundtype: %d,len: %d, pts: %lld,dts: %lld",
                       sound_format, sound_rate, sound_size, sound_type, buffered_len_ - index, pts, pts);
                if(cb_){
                    cb_->on_data(DemuxerCallback::DATA_TYPE_AUDIO, buffered_data_ + index, buffered_len_ - index, cur_tag_timestamp_, cur_tag_timestamp_);
                }
            }
        }else if(sound_format == 11){
            // speex
        }
    }else if(cur_tag_type_ == 9){
        //  video tag
        char header = buffered_data_[0];
        int frame_type = (header & 0xf0) >> 4;
        int codec_id = header & 0x0f;
        index++;
        if(codec_id == 7){
            // AVCVIDEOPACKET
            char avc_packet_type = buffered_data_[index];
            index++;
            unsigned char* _buffer_data = (unsigned char*)buffered_data_;
            int32_t composition_time =    _buffer_data[index] << 16
                                        | _buffer_data[index+1] << 8
                                        | _buffer_data[index+2];
            int64_t dts = cur_tag_timestamp_;
            int64_t pts = cur_tag_timestamp_ + composition_time;
            index += 3;
            if(avc_packet_type == 0){
                // AVCDecoderConfigurationRecord
//                CILOGD("got AVCDecoderConfigurationRecord, len: %d", buffered_len_ - index);
                if(cb_){
                    cb_->on_data(DemuxerCallback::DATA_TYPE_VIDEO_CSD,
                                 buffered_data_ + index, buffered_len_ - index, 0, 0);
                }
            }else if(avc_packet_type == 1){
                // AVC NALU
                CILOGD("got AVC NALU, frametype: %d, codec_id: %d ,cts: %d, len: %d, pts: %lld, dts: %lld",
                       frame_type, codec_id, composition_time, buffered_len_ - index, pts, dts);
                if(cb_){
                    cb_->on_data(DemuxerCallback::DATA_TYPE_VIDEO_CSD,
                                 buffered_data_ + index, buffered_len_ - index, pts, dts);
                }
            }
        }else if(codec_id == 4){
            //  VP6FLVVIDEOPACKET
        }
    }else if(cur_tag_type_ == 18){
        //  metadata TODO
    }
    return 0;
}