#include "net.h"
#include "poller.h"
#include <iostream>

NetEvent::NetEvent(int fd)
    :fd_(fd), events_(0), revents_(0)
{}

NetEvent::~NetEvent()
{
    //TODO:close fd
    //LOG
}

void NetEvent::handleEvent()
{
    //LOG
    if (revents_ & NET_READABLE)
    {
        if (readCallback_) readCallback_();
    }
    if (revents_ & NET_WRITABLE)
    {
        if (writeCallback_) writeCallback_();
    }
}

EventLoop::EventLoop()
    :quit_(false)
{
    //LOG
    poller_ = new EPoller(10000); //最大连接数
    active_events_ = new NetEvent*[1024];
}

EventLoop::~EventLoop()
{
    //LOG
}

void EventLoop::loop()
{
    std::cout << "start EventLoop::loop()" << std::endl;
    while (!quit_)
    {
        int num = poller_->poll(active_events_, 1024, 1);
        for (int i = 0; i < num; ++i)
            active_events_[i]->handleEvent();
        //LOG
    }
}