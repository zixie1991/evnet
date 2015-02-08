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
#include "log.h"

using std::map;
using std::string;

using boost::scoped_ptr;
using boost::shared_ptr;
using boost::bind;


class StreamClient {
    public:
        StreamClient(EventLoop* loop, InetAddress server_addr);
        ~StreamClient();

        void start(int num_clients);

    private:
        void newConnection(int sockfd, const InetAddress& peeraddr);
        void onConnection(const shared_ptr<Connection>& connection);
        void onMessage(const shared_ptr<Connection>& connection, const char* data, int len);
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
    StreamClient stream_client(&loop, server_addr);

    stream_client.start(1000);

    return 0;
}

StreamClient::StreamClient(EventLoop* loop, InetAddress server_addr):
    loop_(loop),
    connector_(new Connector(server_addr)),
    next_connection_id_(1),
    started_(false)
{
    connector_->set_new_connection_callback(bind(&StreamClient::newConnection, this, _1, _2));
}

StreamClient::~StreamClient() {

}

void StreamClient::start(int num_clients) {
    if (started_) {
        return ;
    }

    started_ = true;
    for (int i = 0; i < num_clients; i++) {
        connector_->start();
    }

    loop_->loop();
}

void StreamClient::newConnection(int sockfd, const InetAddress& peeraddr) {
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", next_connection_id_);
    ++next_connection_id_;
    string connection_name = buf;

    log_info("Established a new connection %s with %s:%d", connection_name.c_str(), \
            peeraddr.ip().c_str(), peeraddr.port());

    shared_ptr<Connection> connection(new Connection(loop_, connection_name, \
                sockfd, peeraddr));
    connections_[connection_name] = connection;
    connection->set_connection_callback(bind(&StreamClient::onConnection, this, _1));
    connection->set_message_callback(bind(&StreamClient::onMessage, this, _1, _2, _3));
    connection->set_close_callback(bind(&StreamClient::removeConnection, this, _1));
    connection->connectionEstablished();
}

void StreamClient::onConnection(const shared_ptr<Connection>& connection) {
    // send message "hello, world"
    connection->connectionStreamed();
}

void StreamClient::onMessage(const shared_ptr<Connection>& connection, const char* data, int len) {
    (void)data;
    log_info("message callback: received %d bytes from connection [%s]", \
            len, connection->name().c_str());
}

void StreamClient::removeConnection(const shared_ptr<Connection>& connection) {
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
