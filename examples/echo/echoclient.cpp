#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "eventloop.h"
#include "inetaddress.h"
#include "connector.h"
#include "connection.h"
#include "buffer.h"
#include "log.h"

using std::map;
using std::string;

using boost::scoped_ptr;
using boost::shared_ptr;
using boost::bind;

class EchoClient {
    public:
        EchoClient(EventLoop* loop, InetAddress server_addr);
        ~EchoClient();

        void start(int num_clients);

    private:
        void newConnection(int sockfd, const InetAddress& peeraddr);
        void onConnection(const shared_ptr<Connection>& connection);
        void onMessage(const shared_ptr<Connection>& connection, Buffer& buffer);
        void removeConnection(const shared_ptr<Connection>& connection);

        EventLoop *loop_;
        
        scoped_ptr<Connector> connector_;
        map<string, shared_ptr<Connection> > connections_;
        int next_connection_id_;
    
        bool started_;
};

int main(int argc, char *argv[]) {
    if (3 != argc) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(0);
    }

    set_log_level(Logger::LEVEL_ERROR);
    string ip(argv[1]);
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress server_addr(ip, port);

    EventLoop loop;
    EchoClient echo_client(&loop, server_addr);

    echo_client.start(1000);

    return 0;
}

EchoClient::EchoClient(EventLoop* loop, InetAddress server_addr):
    loop_(loop),
    connector_(new Connector(server_addr)),
    next_connection_id_(1),
    started_(false)
{
    connector_->set_new_connection_callback(bind(&EchoClient::newConnection, this, _1, _2));
}

EchoClient::~EchoClient() {

}

void EchoClient::start(int num_clients) {
    if (started_) {
        return ;
    }

    started_ = true;
    for (int i = 0; i < num_clients; i++) {
        connector_->start();
    }

    loop_->loop();
}

void EchoClient::newConnection(int sockfd, const InetAddress& peeraddr) {
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", next_connection_id_);
    ++next_connection_id_;
    string connection_name = buf;

    log_info("Established a new connection %s with %s:%d", connection_name.c_str(), \
            peeraddr.ip().c_str(), peeraddr.port());

    shared_ptr<Connection> connection(new Connection(loop_, connection_name, \
                sockfd, peeraddr));
    connections_[connection_name] = connection;
    connection->set_connection_callback(bind(&EchoClient::onConnection, this, _1));
    connection->set_message_callback(bind(&EchoClient::onMessage, this, _1, _2));
    connection->set_close_callback(bind(&EchoClient::removeConnection, this, _1));
    connection->connectionEstablished();
}

void EchoClient::onConnection(const shared_ptr<Connection>& connection) {
    // send message "hello, world"
    char buf[65536];
    memset(buf, 'a', sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    connection->send(buf, strlen(buf));
}

void EchoClient::onMessage(const shared_ptr<Connection>& connection, Buffer& buffer) {
    string message;
    buffer.read(message);
    connection->send(message.c_str(), message.size());
}

void EchoClient::removeConnection(const shared_ptr<Connection>& connection) {
    log_info("close callback: remote peer[%s] closed connection", \
            connection->name().c_str());

    log_debug("before connections_.erase(%s): %d connections", connection->name().c_str(), connections_.size());
    size_t n = connections_.erase(connection->name());
    (void)n;
    assert(1 == n);
    log_debug("after connections_.erase(%s): %d connections", connection->name().c_str(), connections_.size());


    // remove channel on next eventloop.loop
    // @code
    // loop_->removeChannel(channel_.get());
    // @endcode
}
