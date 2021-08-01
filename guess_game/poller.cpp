#include "poller.h"
#include "net.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

//最大描述符数
static const int MAX_FD_SIZE = 1024 * 1024;
//epoll一次性最多返回事件数
static const int MAX_EPOLL_SIZE = 1024;

Poller::Poller()
{
    //LOG
    register_events_ = (NetEvent**)calloc(1, sizeof(NetEvent*) * MAX_FD_SIZE);
}

Poller::~Poller()
{
    //LOG
    free(register_events_);
}

EPoller::EPoller()
{
    //LOG
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
    epoll_events_ = (epoll_event*)calloc(1, sizeof(epoll_event) * MAX_EPOLL_SIZE);
}

EPoller::~EPoller()
{
    //LOG
    close(epollfd_);
    free(epoll_events_);
}

bool EPoller::updateNetEvent(NetEvent* event)
{
    assert(event != NULL);
    int fd = event->fd();
    if (fd >= MAX_FD_SIZE)
        return false;

    int op = 0;
    if (NULL == register_events_[fd]) //新增
    {
        op = EPOLL_CTL_ADD;
    }
    else
    {
        assert(fd == register_events_[fd]->fd());
        int events = event->events();
        op = (events == NET_NONE) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    }

    //添加epoll事件
    struct epoll_event ee = {0};
    ee.events = event->events();
    ee.data.ptr = event;
    if (epoll_ctl(epollfd_, op, fd, &ee) == -1)
        return false;
    if (op == EPOLL_CTL_ADD)
        register_events_[fd] = event;
    else if (op == EPOLL_CTL_DEL)
        register_events_[fd] = NULL; //重置
    return true;
}

bool EPoller::delNetEvent(NetEvent* event)
{
    assert(event != NULL);
    int fd = event->fd();
    if (fd >= MAX_FD_SIZE)
        return false;

    struct epoll_event ee = {0};
    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ee) == -1)
        return false;
    register_events_[fd] = NULL; //重置
    return true;
}

int EPoller::poll(NetEvent** active_events, int ac_num, int timeout_ms)
{
    //LOG
    assert(ac_num > 0);
    assert(active_events != NULL);
    int max_events = (ac_num > MAX_EPOLL_SIZE) ? MAX_EPOLL_SIZE : ac_num;
    int num_events = epoll_wait(epollfd_, epoll_events_, max_events, timeout_ms);
    if (num_events > 0)
    {
        for (int i = 0; i < num_events; ++i)
        {
            // 根据就绪的事件类型，设置mask
            // FIXME: 保留具体错误
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