//
//  Reactor_Impl.h
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef Reactor_Impl_h
#define Reactor_Impl_h

#include "Reactor.h"
#include "OSAL.h"

class Demultiplexer;

class Reactor_Impl : public Reactor, public enable_shared_from_this<Reactor_Impl>{
public:
    Reactor_Impl();
    virtual int register_handler(const shared_ptr<ReactorHandler> handler, int mask);
    virtual int remove_handler(const shared_ptr<ReactorHandler> handler, int mask);
    virtual int start();
    virtual int stop();
    
    virtual int init();
    virtual int deInit();
    
private:
    virtual int handle_fds(int handle, int mask);
    
private:
    friend class Reactor;
    shared_ptr<Demultiplexer> demultiplexer_;
    template<class P> struct WeakPtrLess;
    template <class T>
    struct WeakPtrLess< weak_ptr<T> >{
        bool operator() (const weak_ptr<T>& l, const weak_ptr<T>& r) const{
            shared_ptr<T> sl = l.lock();
            shared_ptr<T> sr = r.lock();
            return sl < sr;
        }
    };
    
    typedef map< weak_ptr<ReactorHandler>, int ,WeakPtrLess< weak_ptr<ReactorHandler>> > HANDLER_MAP;
    HANDLER_MAP map_handlers_;
    Mutex mutex_;
    bool started_;
};

#endif /* Reactor_Impl_h */
