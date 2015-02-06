#include "connection.h"

#include <boost/bind.hpp>

#include "socket.h"
#include "channel.h"


Connection::Connection(EventLoop* loop, std::string name, int sockfd, const InetAddress& peeraddr):
    loop_(loop),
    name_(name),
    peeraddr_(peeraddr),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd))
{
    socket_->setNonblock();

    channel_->set_read_callback(boost::bind(&Connection::handleReadEvent, this));
}

Connection::~Connection() {

}

void Connection::connectionEstablished() {
    channel_->enableReadEvent();

    connection_callback_(this);
}

void Connection::handleReadEvent() {
    char buf[4096];
    int n = 0;
    n = ::read(socket_->fd(), buf, sizeof(buf) - 1);
    buf[n] = '\0';

    message_callback_(this, buf, n);
}
