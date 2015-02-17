#include "tcpserver.h"

#include <boost/bind.hpp>

#include "acceptor.h"
#include "log.h"

using boost::bind;
using boost::scoped_ptr;
using boost::shared_ptr;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr):
    loop_(loop),
    acceptor_(new Acceptor(loop, listen_addr)),
    started_(false),
    next_connection_id_(1)
{
    acceptor_->set_new_connection_callback_(bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    if (!started_) {
        started_ = true;

        acceptor_->listen();
        loop_->loop();
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peeraddr) {
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", next_connection_id_);
    ++next_connection_id_;
    string connection_name = buf;

    log_info("Accepted a new connection %s from %s:%d", connection_name.c_str(), \
            peeraddr.ip().c_str(), peeraddr.port());

    shared_ptr<Connection> connection(new Connection(loop_, connection_name, \
                sockfd, peeraddr));
    connections_[connection_name] = connection;
    connection->set_connection_callback(connection_callback_);
    connection->set_message_callback(message_callback_);
    connection->set_close_callback(bind(&TcpServer::removeConnection, this, _1));
    connection->connectionEstablished();
}

void TcpServer::removeConnection(const boost::shared_ptr<Connection>& connection) {
    log_info("close callback: remote peer[%s] closed connection", \
            connection->name().c_str());

    size_t n = connections_.erase(connection->name());
    (void)n;
    assert(1 == n);


    // remove channel on next eventloop.loop
    // @code
    // loop_->removeChannel(channel_.get());
    // @endcode
    loop_->queueInLoop(bind(&Connection::connectionDestroyed, connection));
}
