#include "net_poller.h"
#include "net.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

Poller::Poller(int max_events)
    :max_events_(max_events)
{
    events_ = new NetEvent*[max_events_];
}

Poller::~Poller()
{
    delete[] events_;
}

EPoller::EPoller(int max_events)
    :Poller(max_events), event_num_(1024)
{
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
    epoll_events_ = new epoll_event[event_num_];
    if (epollfd_ < 0)
    {
        //LOG
        delete[] epoll_events_;
    }
}

EPoller::~EPoller()
{
    close(epollfd_);
    delete[] epoll_events_;
}

void EPoller::addNetEvent(NetEvent* event)
{
}

void EPoller::delNetEvent(NetEvent* event)
{
    //TODO
}

void EPoller::update(int operation, NetEvent* event)
{
    struct epoll_event ee = {0};
}

int EPoller::poll(NetEvent** active_events, int ac_num, int timeout_ms)
{
    //LOG
    int max_events = (ac_num > event_num_) ? event_num_ : ac_num;
    int num_events = epoll_wait(epollfd_, epoll_events_, max_events, timeout_ms);
    if (num_events > 0)
    {
        for (int i = 0; i < num_events; ++i)
        {
            // 根据就绪的事件类型，设置mask
            int mask = NET_NONE;
            struct epoll_event *e = epoll_events_ + i;
            if (e->events & EPOLLIN) mask |= NET_READABLE;
            if (e->events & EPOLLOUT) mask |= NET_WRITABLE;
            if (e->events & EPOLLERR) mask |= NET_WRITABLE;
            if (e->events & EPOLLHUP) mask |= NET_WRITABLE;

            NetEvent* net_event = static_cast<NetEvent*>(e->data.ptr);
            net_event->setRevents(mask);
            active_events[i] = net_event;
        }
        //LOG
    }
    //LOG
    return num_events;
}