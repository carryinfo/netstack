//
//  OSAL.h
//  httpclient
//
//  Created by wang xiaoming on 7/21/16.
//  Copyright © 2016 wang xiaoming. All rights reserved.
//

#ifndef OSAL_h
#define OSAL_h

#include <vector>
#include <map>
#include <stdint.h>
using std::map;
using std::vector;
#include <pthread.h>
#include <sys/time.h>
#include <memory>
#include <string>
using std::string;
using std::shared_ptr;
using std::weak_ptr;

#	define CILOGE(...) printf(__VA_ARGS__);printf("\n");
#	define CILOGW(...) printf(__VA_ARGS__);printf("\n");
#	define CILOGI(...) printf(__VA_ARGS__);printf("\n");
#	define CILOGD(...) printf(__VA_ARGS__);printf("\n");
#	define CILOGV(...) printf(__VA_ARGS__);printf("\n");

class Mutex {
    friend class Condition;
public:
    Mutex(){
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    ~Mutex(){pthread_mutex_destroy(&mMutex);}
    
    int lock(){return -pthread_mutex_lock(&mMutex);}
    void unlock(){pthread_mutex_unlock(&mMutex);}
    
private:
//    Mutex(const Mutex&);
//    Mutex& operator = (const Mutex&);
    
    pthread_mutex_t mMutex;
};

class Autolock {
public:
    inline Autolock(Mutex& mutex) : mLock(mutex)  { mLock.lock(); }
    inline Autolock(Mutex* mutex) : mLock(*mutex) { mLock.lock(); }
    inline ~Autolock() { mLock.unlock(); }
private:
    Mutex& mLock;
};

class Condition {
public:
    Condition(){pthread_cond_init(&mCond, NULL);}
    ~Condition(){pthread_cond_destroy(&mCond);}
    int wait(Mutex& mutex){return -pthread_cond_wait(&mCond, &mutex.mMutex);}
    //  1 000 000 000   ==>  1 s
    int waitRelative(Mutex& mutex, int64_t reltime){
        if(reltime==0){
            return wait(mutex);
        }
        struct timeval t;
        struct timespec ts;
        gettimeofday(&t, NULL);
        ts.tv_sec = t.tv_sec;
        ts.tv_nsec= t.tv_usec*1000;
        ts.tv_sec += reltime/1000000000;
        ts.tv_nsec+= reltime%1000000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec  += 1;
        }
        return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
    }
    void signal(){pthread_cond_signal(&mCond);}
    void broadcast(){pthread_cond_broadcast(&mCond);}
    
private:
    pthread_cond_t mCond;
};



class Thread{
public:
    static void* thread_func(void* arg){
        if(arg){
            Thread* thread = (Thread*)arg;
            pthread_setname_np(thread->name_);
            CILOGD("thread: %s, begin", thread->name_);
            thread->thread_loop();
            CILOGD("thread: %s, end", thread->name_);
        }
        return (void*)NULL;
    }
    Thread():name_(NULL){}
    int run(const char* name){
        name_ = name;
        pthread_create(&tid_, NULL, thread_func, this);
        return 0;
    }
    
    int requestExitAndJoin(){
        pthread_join(tid_, NULL);
        return 0;
    }
    
protected:
    virtual bool thread_loop() = 0;
    
private:
    const char* name_;
    pthread_t tid_;
};

#endif /* OSAL_h */
