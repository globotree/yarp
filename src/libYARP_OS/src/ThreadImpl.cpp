// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
* Author: Lorenzo Natale, Paul Fitzpatrick, Anne van Rossum
* Copyright (C) 2006 The Robotcub consortium
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

// added threadRelease/threadInit methods, synchronization and
// init failure notification -nat

#include <yarp/os/impl/ThreadImpl.h>
#include <yarp/os/impl/SemaphoreImpl.h>
#include <yarp/os/impl/Logger.h>
#include <yarp/os/NetType.h>
#include <yarp/os/impl/PlatformThread.h>
#include <yarp/os/impl/PlatformSignal.h>
#include <yarp/os/impl/PlatformStdlib.h>

#include <stdlib.h>

using namespace yarp::os::impl;

int ThreadImpl::threadCount = 0;
int ThreadImpl::defaultStackSize = 0;
SemaphoreImpl *ThreadImpl::threadMutex = NULL;

void ThreadImpl::init() {
    if (!threadMutex) threadMutex = new SemaphoreImpl(1);
}

void ThreadImpl::fini() {
    if (threadMutex) {
        delete threadMutex;
        threadMutex = NULL;
    }
}

#ifdef __WIN32__
static unsigned __stdcall theExecutiveBranch (void *args)
#else
    PLATFORM_THREAD_RETURN theExecutiveBranch (void *args)
#endif
{
    // just for now -- rather deal with broken pipes through normal procedures
    ACE_OS::signal(SIGPIPE, SIG_IGN);


    /*
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGCHLD);
    ACE_OS::thr_sigsetmask(SIG_BLOCK, &set, NULL);
    fprintf(stderr, "Blocking signals\n");
    */

    ThreadImpl *thread = (ThreadImpl *)args;

    YARP_DEBUG(Logger::get(),"Thread starting up");

    bool success=thread->threadInit();
    thread->notify(success);
    thread->notifyOpened(success);
    thread->synchroPost();

    if (success)
    {
        thread->setPriority();
        thread->run();
        thread->threadRelease();
    }


    //YARP_ERROR(Logger::get(),String("uncaught exception in thread: ") +
    //             e.toString());

    ThreadImpl::changeCount(-1);
    YARP_DEBUG(Logger::get(),"Thread shutting down");
    //ACE_Thread::exit();

    thread->notify(false);
    thread->synchroPost();

    return 0;
}


ThreadImpl::ThreadImpl() : synchro(0) {
    delegate = NULL;
    active = false;
    closing = false;
    needJoin = false;
    initWasSuccessful = false;
    defaultPriority = -1;
    defaultPolicy = -1;
    setOptions();
}


ThreadImpl::ThreadImpl(Runnable *target) : synchro(0) {
    delegate = target;
    active = false;
    closing = false;
    needJoin = false;
    defaultPriority = -1;
    defaultPolicy = -1;
    setOptions();
}


ThreadImpl::~ThreadImpl() {
    YARP_DEBUG(Logger::get(),"Thread being deleted");
    join();
}


long int ThreadImpl::getKey() {
    // if id doesn't fit in long int, should do local translation
    return (long int)id;
}

long int ThreadImpl::getKeyOfCaller() {
#ifdef YARP_HAS_ACE
    return (long int)ACE_Thread::self();
#else
    return (long int)pthread_self();
#endif
}


void ThreadImpl::setOptions(int stackSize) {
    this->stackSize = stackSize;
}

int ThreadImpl::join(double seconds) {
    closing = true;
    if (needJoin) {
        if (seconds>0) {
            if (!initWasSuccessful) {
                // join called before start completed
                YARP_ERROR(Logger::get(),String("Tried to join a thread before starting it"));
                return -1;
            }
            synchro.waitWithTimeout(seconds);
            if (active) return -1;
        }
        int result = PLATFORM_THREAD_JOIN(hid);
        needJoin = false;
        active = false;
        while (synchro.check()) {}
        return result;
    }
    return 0;
}

void ThreadImpl::run() {
    if (delegate!=NULL) {
        delegate->run();
    }
}

void ThreadImpl::close() {
    closing = true;
    if (delegate!=NULL) {
        delegate->close();
    }
    join(-1);
}

// similar to close(), but does not join (does not block)
void ThreadImpl::askToClose() {
    closing = true;
    if (delegate!=NULL) {
        delegate->close();
    }
}

void ThreadImpl::beforeStart() {
    if (delegate!=NULL) {
        delegate->beforeStart();
    }
}

void ThreadImpl::afterStart(bool success) {
    if (delegate!=NULL) {
        delegate->afterStart(success);
    }
}

bool ThreadImpl::threadInit()
{
    if (delegate!=NULL){
        return delegate->threadInit();
    }
    else
        return true;
}

void ThreadImpl::threadRelease()
{
    if (delegate!=NULL){
        delegate->threadRelease();
    }
}

bool ThreadImpl::start() {
    join();
    closing = false;
    initWasSuccessful = false;
    beforeStart();
#ifdef YARP_HAS_CXX11
    hid = std::thread(theExecutiveBranch,(void*)this);
    id = 0;
    int result = hid.joinable()?0:1;
#else
#  ifdef YARP_HAS_ACE
    size_t s = stackSize;
    if (s==0) {
        s = (size_t)defaultStackSize;
    }
    int result = ACE_Thread::spawn((ACE_THR_FUNC)theExecutiveBranch,
                                   (void *)this,
                                   THR_JOINABLE | THR_NEW_LWP,
                                   &id,
                                   &hid,
                                   ACE_DEFAULT_THREAD_PRIORITY,
                                   0,
                                   s);
#  else
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
    if (s != 0)
        perror("pthread_attr_init");

    if (stackSize > 0) {
        s = pthread_attr_setstacksize(&attr, stackSize);
        if (s != 0)
            perror("pthread_attr_setstacksize");
    }

    int result = pthread_create(&hid, &attr, theExecutiveBranch, (void*)this);
    id = (long int) hid;
#  endif
#endif

    if (result==0)
    {
        // we must, at some point in the future, join the thread
        needJoin = true;

        // the thread started correctly, wait for the initialization
        YARP_DEBUG(Logger::get(), String("Child thread initializing"));
        synchroWait();
        initWasSuccessful = true;
        if (opened)
        {
            ThreadImpl::changeCount(1);
            YARP_DEBUG(Logger::get(),"Child thread initialized ok");
            afterStart(true);
            return true;
        }
        else
        {
            YARP_DEBUG(Logger::get(),"Child thread did not initialize ok");
            //wait for the thread to really exit
            ThreadImpl::join(-1);
        }
    }
    //the thread did not start, call afterStart() to warn the user
    char tmp[80];
    sprintf(tmp, "%d", result);
    YARP_ERROR(Logger::get(),String("A thread failed to start with error code: ")+String(tmp));
    afterStart(false);
    return false;
}

void ThreadImpl::synchroWait()
{
    synchro.wait();
}

void ThreadImpl::synchroPost()
{
    synchro.post();
}

void ThreadImpl::notify(bool s)
{
    active=s;
}

bool ThreadImpl::isClosing() {
    return closing;
}

int ThreadImpl::getCount() {
    init();
    threadMutex->wait();
    int ct = threadCount;
    threadMutex->post();
    return ct;
}


void ThreadImpl::changeCount(int delta) {
    init();
    threadMutex->wait();
    threadCount+=delta;
    threadMutex->post();
}

int ThreadImpl::setPriority(int priority, int policy) {
    if (priority==-1) {
        priority = defaultPriority;
        policy = defaultPolicy;
    } else {
        defaultPriority = priority;
        defaultPolicy = policy;
    }
    if (active && priority!=-1) {

#if defined(YARP_HAS_CXX11)
        YARP_ERROR(Logger::get(),"Cannot set priority with C++11");
#else
    #if defined(YARP_HAS_ACE) // Use ACE API
        return ACE_Thread::setprio(hid, priority, policy);
    #elif defined(UNIX) // Use the POSIX syscalls
        struct sched_param thread_param;
        thread_param.sched_priority = priority;
        int ret pthread_setschedparam(hid, policy, &thread_param);
        return (ret != 0) ? -1 : 0;
    #else
        YARP_ERROR(Logger::get(),"Cannot set priority without ACE");
    #endif
#endif
    }
    return 0;
}

int ThreadImpl::getPriority() {
    int prio = defaultPriority;
    if (active) {
#if defined(YARP_HAS_CXX11)
        YARP_ERROR(Logger::get(),"Cannot get priority with C++11");
#else
    #if defined(YARP_HAS_ACE) // Use ACE API
        ACE_Thread::getprio(hid, prio);
    #elif defined(UNIX) // Use the POSIX syscalls
        struct sched_param thread_param;
        int policy;
        if(pthread_getschedparam(hid, &policy, &thread_param) == 0)
            prio = thread_param.priority;
    #else
        YARP_ERROR(Logger::get(),"Cannot read priority without ACE");
    #endif
#endif
    }
    return prio;
}

int ThreadImpl::getPolicy() {
    int policy = defaultPolicy;
    int prio;
    if (active) {
#if defined(YARP_HAS_CXX11)
        YARP_ERROR(Logger::get(),"Cannot get scheduiling policy with C++11");
#else
    #if defined(YARP_HAS_ACE) // Use ACE API
        ACE_Thread::getprio(hid, prio, policy);
    #elif defined(UNIX) // Use the POSIX syscalls
        struct sched_param thread_param;
        if(pthread_getschedparam(hid, &policy, &thread_param) != 0)
            policy = defaultPolicy;
    #else
        YARP_ERROR(Logger::get(),"Cannot read scheduling policy without ACE");
    #endif
#endif
    }
    return policy;
}


void ThreadImpl::setDefaultStackSize(int stackSize) {
    defaultStackSize = stackSize;
}
