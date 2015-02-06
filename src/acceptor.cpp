#include "acceptor.h"

#include <boost/bind.hpp>

#include "inetaddress.h"

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr):
    loop_(loop),
    channel_(loop, socket_.generateSocket()),
    listenning_(false)
{
    socket_.bindAddress(listen_addr);
    socket_.setReuseAddr(true);

    channel_.enableReadEvent();
    channel_.set_read_callback(boost::bind(&Acceptor::handleReadEvent, this));
}

Acceptor::~Acceptor() {

}

void Acceptor::set_new_connection_callback_(NewConnectionCallback cb) {
    new_connection_callback_ = cb;
}

void Acceptor::listen() {
    listenning_ = true;
    socket_.listen();
}

void Acceptor::handleReadEvent() {
    InetAddress peeraddr;

    int connfd = socket_.accept(peeraddr);
    if (connfd >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peeraddr);
        } else {
            ::close(connfd);
        }
    } 
}
