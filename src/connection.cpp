#include "connection.h"

#include <boost/bind.hpp>

#include "socket.h"
#include "channel.h"
#include "log.h"


Connection::Connection(EventLoop* loop, std::string name, int sockfd, const InetAddress& peeraddr):
    loop_(loop),
    name_(name),
    peeraddr_(peeraddr),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd))
{
    log_debug("Connection constructor: name:%s, fd:%d", name_.c_str(), socket_->fd());
    channel_->set_read_callback(boost::bind(&Connection::handleReadEvent, this));
}

Connection::~Connection() {
    log_debug("Connection destructor: name=%s, fd=%d", name_.c_str(), socket_->fd());
}

void Connection::connectionEstablished() {
    channel_->enableReadEvent();
    //connection_callback_(this);
}

void Connection::set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
}

void Connection::set_message_callback(const MessageCallback& cb) {
    message_callback_ = cb;
}

void Connection::handleReadEvent() {
    char buf[4096];
    int n = 0;
    n = ::read(socket_->fd(), buf, sizeof(buf) - 1);
    buf[n] = '\0';

    log_debug("handle read event: received %d bytes: %s\n", n, buf);
    message_callback_(this, buf, n);
}
