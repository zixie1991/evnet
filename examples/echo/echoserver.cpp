#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "eventloop.h"
#include "inetaddress.h"
#include "connection.h"
#include "tcpserver.h"
#include "buffer.h"
#include "log.h"

using boost::shared_ptr;
using boost::bind;

class EchoServer {
    public:
        EchoServer(EventLoop* loop, const InetAddress& listen_addr);
        ~EchoServer();

        void start();

    private:
        void onConnection(const shared_ptr<Connection>& connection);
        void onMessage(const shared_ptr<Connection>& connection, Buffer& buffer);

        TcpServer server_;
};

int main(int argc, char *argv[]) {
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    set_log_level(Logger::LEVEL_TRACE);
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress listen_addr(port);
    EventLoop loop;

    EchoServer echo_server(&loop, listen_addr);
    echo_server.start();

    return 0;
}

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listen_addr):
    server_(loop, listen_addr)
{
    server_.set_connection_callback(bind(&EchoServer::onConnection, this, _1));
    server_.set_message_callback(bind(&EchoServer::onMessage, this, _1, _2));
}

EchoServer::~EchoServer() {

}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const shared_ptr<Connection>& connection) {
    log_info("connection callback: new connection [%s] from %s:%d", \
            connection->name().c_str(), connection->peeraddr().ip().c_str(), \
            connection->peeraddr().port());
}

void EchoServer::onMessage(const shared_ptr<Connection>& connection, Buffer& buffer) {
    log_info("message callback: received %d bytes from connection [%s]", \
            buffer.readableBytes(), connection->name().c_str());

    string message;
    buffer.read(message);
    connection->send(message.c_str(), message.size());
}
