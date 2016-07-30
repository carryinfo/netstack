//
//  Demultiplexer_select.h
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef Demultiplexer_select_h
#define Demultiplexer_select_h

#include "Reactor.h"
#include "OSAL.h"

class Demultiplexer{
public:
    virtual ~Demultiplexer(){}
    virtual int register_handler(int handler, int mask) = 0;
    virtual int remove_handler(int handler, int mask) = 0;
    
    virtual int start() = 0;
    virtual int stop() = 0;
};

class Demultiplexer_select : public Demultiplexer, public Thread{
public:
    Demultiplexer_select(shared_ptr<Reactor> reactor);
    virtual int register_handler(int handler, int mask);
    virtual int remove_handler(int handler, int mask);
    virtual int start();
    virtual int stop();
protected:
    virtual bool thread_loop();
    
private:
    int demultiplexer_run();
    int event_run();
    
private:
    typedef vector<int> FD_Set;
    FD_Set r_set_, w_set_, e_set_;
    Mutex mutex_;
    weak_ptr<Reactor> reactor_;
    bool started_;
};


#endif /* Demultiplexer_select_h */
