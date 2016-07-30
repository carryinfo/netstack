//
//  Demultiplexer_select.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "Demultiplexer_select.h"

Demultiplexer_select::Demultiplexer_select(shared_ptr<Reactor> reactor) : started_(false){
    reactor_ = reactor;
}

int Demultiplexer_select::register_handler(int handler, int mask){
    Autolock _LOCK(mutex_);
    if(mask & MASK_READ){
        FD_Set::const_iterator iter = std::find(r_set_.begin(), r_set_.end(), handler);
        if(iter == r_set_.end()){
            r_set_.push_back(handler);
        }
    }
    
    if(mask & MASK_WRITE){
        FD_Set::const_iterator iter = std::find(w_set_.begin(), w_set_.end(), handler);
        if(iter == w_set_.end()){
            w_set_.push_back(handler);
        }
    }
    
    if(mask & MASK_EXCEPTION){
        FD_Set::const_iterator iter = std::find(e_set_.begin(), e_set_.end(), handler);
        if(iter == e_set_.end()){
            e_set_.push_back(handler);
        }
    }
    return 0;
}

int Demultiplexer_select::remove_handler(int handler, int mask){
    Autolock _LOCK(mutex_);
    if(mask & MASK_READ){
        FD_Set::const_iterator iter = std::find(r_set_.begin(), r_set_.end(), handler);
        if(iter != r_set_.end()){
            r_set_.erase(iter);
        }
    }
    
    if(mask & MASK_WRITE){
        FD_Set::const_iterator iter = std::find(w_set_.begin(), w_set_.end(), handler);
        if(iter != w_set_.end()){
            w_set_.erase(iter);
        }
    }
    
    if(mask & MASK_EXCEPTION){
        FD_Set::const_iterator iter = std::find(e_set_.begin(), e_set_.end(), handler);
        if(iter != e_set_.end()){
            e_set_.erase(iter);
        }
    }
    return 0;
}

int Demultiplexer_select::start(){
    Autolock _LOCK(mutex_);
    if(!started_){
        Thread::run("Demultiplexer_select");
    }
    return 0;
}

int Demultiplexer_select::stop(){
    Autolock _LOCK(mutex_);
    started_ = false;
    Thread::requestExitAndJoin();
    return 0;
}


bool Demultiplexer_select::thread_loop(){
    started_ = true;
    
    while(started_){
        demultiplexer_run();
        event_run();
    }
    
    return true;
}

int Demultiplexer_select::demultiplexer_run(){
    Autolock _LOCK(mutex_);
    int max_fd = 0;
    fd_set fd_read, fd_write, fd_except;
    int millisecond = 40;
    struct timeval tv = {0};
    tv.tv_usec = (millisecond % 1000) * 1000;
    FD_ZERO(&fd_read);
    for(int i=0; i< r_set_.size(); i++){
        FD_SET(r_set_[i], &fd_read);
        max_fd = (r_set_[i] > max_fd? r_set_[i] : max_fd);
    }
    FD_ZERO(&fd_write);
    for(int i=0; i< w_set_.size(); i++){
        FD_SET(w_set_[i], &fd_write);
        max_fd = (w_set_[i] > max_fd? w_set_[i] : max_fd);
    }
    FD_ZERO(&fd_except);
    for(int i=0; i< e_set_.size(); i++){
        FD_SET(e_set_[i], &fd_except);
        max_fd = (e_set_[i] > max_fd? e_set_[i] : max_fd);
    }
    
    max_fd += 1;
    
    int ret = select(max_fd, &fd_read, &fd_write, &fd_except, &tv);
    if (ret == 0) {
        //  timeout
        return 0;
    } else if (ret < 0) {
        return 0;
    } else if (ret > 0) {
        if(started_){
            for(int i=0; i< r_set_.size(); i++){
                int fd = r_set_[i];
                if(FD_ISSET(fd, &fd_read)){
                    if(reactor_.lock()){
                        reactor_.lock()->handle_fds(fd, MASK_READ);
                    }
                }
            }
            
            for(int i=0; i< w_set_.size(); i++){
                int fd = w_set_[i];
                if(FD_ISSET(fd, &fd_write)){
                    if(reactor_.lock()){
                        reactor_.lock()->handle_fds(fd, MASK_WRITE);
                    }
                }
            }
            
            for(int i=0; i< e_set_.size(); i++){
                int fd = e_set_[i];
                if(FD_ISSET(fd, &fd_except)){
                    if(reactor_.lock()){
                        reactor_.lock()->handle_fds(fd, MASK_EXCEPTION);
                    }
                }
            }
        }
    }
    return 0;
}

int Demultiplexer_select::event_run(){
    return 0;
}
