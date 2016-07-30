//
//  Reactor_Impl.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "Reactor_Impl.h"

#include "Demultiplexer_select.h"

Reactor_Impl::Reactor_Impl() : started_(false){
}

int Reactor_Impl::register_handler(const shared_ptr<ReactorHandler> handler, int mask){
    Autolock _LOCK(mutex_);
    HANDLER_MAP::iterator iter = map_handlers_.find(handler);
    if(iter != map_handlers_.end()){
        iter->second |= mask;
    }else{
        map_handlers_.insert(std::pair< weak_ptr<ReactorHandler>, int >(handler,mask));
    }
    if(demultiplexer_){
        demultiplexer_->register_handler(handler->get_handler(), mask);
    }
    return 0;
}

int Reactor_Impl::remove_handler(const shared_ptr<ReactorHandler> handler, int mask){
    Autolock _LOCK(mutex_);
    HANDLER_MAP::iterator iter = map_handlers_.find(handler);
    
    if(iter != map_handlers_.end()){
        iter->second ^= mask;
        if(demultiplexer_){
            demultiplexer_->remove_handler(handler->get_handler(), mask);
        }
        if(iter->second == 0){
            map_handlers_.erase(iter);
        }
    }
    return 0;
}

int Reactor_Impl::start(){
    Autolock _LOCK(mutex_);
    if(!started_){
        if(demultiplexer_){
            demultiplexer_->start();
        }
        started_ = true;
    }
    
    
    return 0;
}

int Reactor_Impl::stop(){
    Autolock _LOCK(mutex_);
    if(started_){
        if(demultiplexer_){
            demultiplexer_->stop();
        }
        started_ = false;
    }
    
    return 0;
}

int Reactor_Impl::handle_fds(int handle, int mask){
    Autolock _LOCK(mutex_);
    HANDLER_MAP::const_iterator iter = map_handlers_.begin();
    for(;iter != map_handlers_.end(); iter++){
        shared_ptr<ReactorHandler> ptr = iter->first.lock();
        if(ptr->get_handler() == handle
           && (iter->second & mask)){
            if(ptr){
                ptr->handle_fds(mask);
            }
        }
    }
    return 0;
}

int Reactor_Impl::init(){
    if(!demultiplexer_){
        demultiplexer_.reset(new Demultiplexer_select(shared_from_this()));
    }
    return 0;
}

int Reactor_Impl::deInit(){
    return 0;
}

