#include "connector.h"

#include <errno.h>

#include <boost/bind.hpp>

#include "socket.h"
#include "channel.h"
#include "log.h"

Connector::Connector(const InetAddress& server_addr):
    server_addr_(server_addr),
    sockfd_(-1),
    connect_(false)
{
}

Connector::~Connector() {

}

void Connector::start() {
    connect_ = true; 
    connect();
}

void Connector::restart() {

}

void Connector::stop() {

}

void Connector::connect() {
    if ((sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    }

    int ret = 0;
    ret = ::connect(sockfd_, (struct sockaddr *)&server_addr_.addr(), sizeof(server_addr_.addr()));
    if (ret < 0) {
        log_error("connect to server %s:%d error", server_addr_.ip().c_str(), \
                server_addr_.port());
    } else {
        connecting();
    }
}

void Connector::connecting() {
    new_connection_callback_(sockfd_, server_addr_);
}

void Connector::set_new_connection_callback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
}
