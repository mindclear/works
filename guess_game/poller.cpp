#include "poller.h"
#include "net.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

Poller::Poller(int max_events)
    :max_events_(max_events)
{
    register_events_ = new NetEvent*[max_events_];
}

Poller::~Poller()
{
    delete[] register_events_;
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

bool EPoller::addNetEvent(NetEvent* event)
{
    assert(event != NULL);
    int fd = event->fd();
    if (fd >= max_events_)
        return false;

    int events = register_events_[fd]->events();
    int op = (events == NET_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    //添加修改事件
    struct epoll_event ee = {0};
    ee.events = events;
    ee.data.ptr = event;
    if (epoll_ctl(epollfd_, op, fd, &ee) == -1)
        return false;
    return true;
}

bool EPoller::delNetEvent(NetEvent* event)
{
    assert(event != NULL);
    int fd = event->fd();
    if (fd >= max_events_)
        return false;

    struct epoll_event ee = {0};
    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ee) == -1)
        return false;
    return true;
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