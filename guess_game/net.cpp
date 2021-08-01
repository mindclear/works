#include "net.h"
#include "poller.h"
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

NetEvent::NetEvent(int fd)
    :fd_(fd), events_(0), revents_(0)
{
    //LOG
}

NetEvent::~NetEvent()
{
    //LOG
}

void NetEvent::handleEvent()
{
    //FIXME:
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
    :quit_(false), head_functor_(NULL), tail_functor_(NULL)
{
    //LOG
    poller_ = new EPoller();
    active_events_ = (NetEvent**)calloc(1, sizeof(NetEvent*) * 1024);
}

EventLoop::~EventLoop()
{
    //LOG
    delete poller_;
    free(active_events_);
}

void EventLoop::loop()
{
    std::cout << "start EventLoop::loop()" << std::endl;
    while (!quit_)
    {
        int num = poller_->poll(active_events_, 1024, 60 * 1000);
        for (int i = 0; i < num; ++i)
            active_events_[i]->handleEvent();
        //LOG
        doFunctors();
    }
}

void EventLoop::updateNetEvent(NetEvent* event)
{
    //LOG
    poller_->updateNetEvent(event);
}

void EventLoop::queueInLoop(Functor cb)
{
    FunctorNode* node = new FunctorNode();
    node->functor = cb;
    if (head_functor_ == NULL)
    {
        head_functor_ = node;
        tail_functor_ = node;
    }
    else
    {
        tail_functor_->next = node;
        tail_functor_ = node;
    }
}

void EventLoop::doFunctors()
{
    FunctorNode* tmp = NULL;
    FunctorNode* cur = head_functor_;
    //提前设置NULL，防止call的过程中添加
    head_functor_ = NULL;
    tail_functor_ = NULL;
    while (cur)
    {
        cur->functor();
        tmp = cur;
        cur = cur->next;
        delete tmp;
    }
}

TcpConnection::TcpConnection(EventLoop* loop, int fd)
    :loop_(loop), fd_(fd), context_(NULL), connect_(true)
{
    //LOG
    event_ = new NetEvent(fd);
    event_->setReadCallback(std::bind(&TcpConnection::handleRead, this)); //FIXME:
    event_->enableReading();
    loop_->updateNetEvent(event_);
}

TcpConnection::~TcpConnection()
{
    //LOG
    delete event_;
}

void TcpConnection::send(const void* data, int nlen)
{
    //当前没有数据，先尝试直接write
    if (!event_->isWriting() && output_buffer_.readableBytes() == 0)
    {
        int nwrote = write(fd_, data, nlen);
        if (nwrote >= 0)
        {
            nlen -= nwrote;
        }
        else
        {
            //LOG
        }
    }
    if (nlen > 0)
    {
        //写入缓冲区，并关注写事件
        output_buffer_.append((const char*)data, nlen);
        if (!event_->isWriting())
        {
            event_->enableWriting();
            loop_->updateNetEvent(event_);
        }
    }
}

void TcpConnection::handleRead()
{
    int nread = input_buffer_.readFd(fd_);
    if (nread > 0)
    {
        //LOG
        messageCallback_(this, &input_buffer_);
    }
    else if (nread == 0) //对方关闭连接
    {
        handleClose();
    }
    else
    {
        //LOG
    }
}

void TcpConnection::handleClose()
{
    connect_ = false;
    event_->disableAll();
    loop_->updateNetEvent(event_);
    loop_->queueInLoop(std::bind(closeCallback_, this)); //删除tcp connection
}

TcpServer::TcpServer(EventLoop* loop, uint16_t port)
    :loop_(loop), listen_port_(port), listen_fd_(-1), listen_event_(NULL)
{
    assert(loop_ != NULL);
    //LOG
}

TcpServer::~TcpServer()
{
    //LOG
    delete listen_event_;
}

bool TcpServer::start()
{
    //LOG
    listen_fd_ = createTcpSocket(listen_port_);
    if (listen_fd_ < 0 )
    {
        //LOG
        return false;
    }

    int ret = listen(listen_fd_, SOMAXCONN);
    if (ret < 0)
    {
        //LOG
        return false;
    }

    //创建监听事件
    listen_event_ = new NetEvent(listen_fd_);
    listen_event_->setReadCallback(std::bind(&TcpServer::handleAccept, this));
    listen_event_->enableReading();
    loop_->updateNetEvent(listen_event_);
    //LOG
    return true;
}

int TcpServer::createTcpSocket(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (fd < 0)
        return fd;

    if (port != 0)
    {
        //FIXME:检查返回值
        int optval = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
	    addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind(fd, (struct sockaddr*)&addr, static_cast<socklen_t>(sizeof(addr)));
    }
	return fd;
}

void TcpServer::handleAccept()
{
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    memset(&sa, 0, salen);
    int connfd = accept(listen_fd_, (struct sockaddr*)&sa, &salen);
    if (connfd >= 0)
    {
        char peer_ip[32] = {0};
        uint16_t peer_port = 0;

        if (sa.sin_family == AF_INET)
        {
            inet_ntop(AF_INET, (void*)&(sa.sin_addr), peer_ip, 32);
            peer_port = ntohs(sa.sin_port);
            //LOG
            newConnection(connfd, peer_ip, peer_port);
        }
        else
        {
            close(connfd);
            //LOG
        }
    }
}

void TcpServer::newConnection(int connfd, const char* peer_ip, const uint16_t peer_port)
{
    //LOG
    //FIXME:提供接口设置
    int optval = 1;
    setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));

    //新建连接
    TcpConnection* conn = new TcpConnection(loop_, connfd);
    conn->setMessageCallback(messageCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    connectionCallback_(conn);
}

void TcpServer::removeConnection(TcpConnection* conn)
{
    //LOG
    //FIXME:
    delete conn;
}