#include "connector.h"


Connector::Connector(EventLoop* loop, const InetAddress& server_addr):
    loop_(loop),
    server_addr_(server_addr),
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
}

void Connector::connecting(int sockfd) {

}

void Connector::handleWriteEvent() {

}

void Connector::handleErrorEvent() {

}

void Connector::set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
}
