
//
//  Reactor.h
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef Reactor_h
#define Reactor_h

enum EVENT_MASK{
    MASK_READ = 1 << 0,
    MASK_WRITE = 1 << 1,
    MASK_EXCEPTION = 1 << 2,
};

#include <memory>
using std::shared_ptr;
using std::enable_shared_from_this;

class ReactorHandler : public enable_shared_from_this<ReactorHandler>{
public:
    virtual int handle_fds(int mask) =  0;
    virtual int get_handler() = 0;
};

class Reactor{
public:
    virtual ~Reactor(){}
    static shared_ptr<Reactor> getReactor();
    virtual int init() = 0;
    virtual int deInit() = 0;
    virtual int register_handler(const shared_ptr<ReactorHandler> handler, int mask) = 0;
    virtual int remove_handler(const shared_ptr<ReactorHandler> handler, int mask) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
private:
    friend class Demultiplexer_select;
    virtual int handle_fds(int handle, int mask) = 0;
};

#endif /* Reactor_h */
