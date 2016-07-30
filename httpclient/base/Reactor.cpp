//
//  Reactor.cpp
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#include "Reactor.h"
#include "OSAL.h"
#include "Reactor_Impl.h"

shared_ptr<Reactor> g_reactor;

shared_ptr<Reactor> Reactor::getReactor(){
    if(!g_reactor){
        g_reactor.reset(new Reactor_Impl);
    }
    return g_reactor;
}
