#include "tcpclient.h"

#include <string>

#include <boost/bind.hpp>

#include "connector.h"
#include "connection.h"
#include "eventloop.h"
#include "log.h"

using boost::bind;
using boost::shared_ptr;
using std::string;

TcpClient::TcpClient(EventLoop* loop, const InetAddress server_addr):
    loop_(loop),
    connector_(new Connector(server_addr))
{
    connector_->set_new_connection_callback(bind(&TcpClient::newConnection, this, _1, _2));
}

TcpClient::~TcpClient() {

}

void TcpClient::connect() {
    connector_->start();
}

void TcpClient::disconnect() {
    if (connection_) {
        connection_->shutdown();
    }
}

void TcpClient::newConnection(int sockfd, const InetAddress& server_addr) {
    char buf[32];
    snprintf(buf, sizeof(buf), "#%s:%d", connector_->server_addr().ip().c_str(), \
            connector_->server_addr().port());
    string connection_name = buf;
    shared_ptr<Connection> connection(new Connection(loop_, connection_name, sockfd, server_addr));
    connection_ = connection;
    connection->set_connection_callback(connection_callback_);
    connection->set_message_callback(message_callback_);
    connection->set_close_callback(bind(&TcpClient::removeConnection, this, _1));

    connection->connectionEstablished();
}

void TcpClient::removeConnection(const boost::shared_ptr<Connection>& connection) {
    log_info("close callback: remote peer[%s] closed connection", \
            connection->name().c_str());

    // remove channel on next eventloop.loop
    // @code
    // loop_->removeChannel(channel_.get());
    // @endcode
    loop_->queueInLoop(bind(&Connection::connectionDestroyed, connection_));
}
