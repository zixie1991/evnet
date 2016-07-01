#include "tcpclient.h"

#include <string>

#include <boost/bind.hpp>

#include "util/log.h"

#include "connector.h"
#include "connection.h"
#include "eventloop.h"

using boost::bind;
using boost::shared_ptr;
using std::string;

TcpClient::TcpClient(EventLoop* loop, const InetAddress server_addr):
  loop_(loop),
  connector_(new Connector(server_addr))
{
  connector_->set_new_connection_callback(bind(&TcpClient::NewConnection, this, _1, _2));
}

TcpClient::~TcpClient() {

}

void TcpClient::Connect() {
  connector_->Start();
}

void TcpClient::Disconnect() {
  if (connection_) {
    connection_->Shutdown();
  }
}

void TcpClient::NewConnection(int sockfd, const InetAddress& server_addr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "#%s:%d", connector_->server_addr().ip().c_str(), \
          connector_->server_addr().port());
  string connection_name = buf;
  shared_ptr<Connection> connection(new Connection(loop_, connection_name, sockfd, server_addr));
  connection_ = connection;
  connection->set_connection_callback(connection_callback_);
  connection->set_message_callback(message_callback_);
  connection->set_write_complete_callback(write_complete_callback_);
  connection->set_close_callback(bind(&TcpClient::RemoveConnection, this, _1));

  connection->ConnectionEstablished();
}

void TcpClient::RemoveConnection(const boost::shared_ptr<Connection>& connection) {
  log_info("Close callback: remote peer[%s] closed connection", \
          connection->name().c_str());

  // remove channel on next eventloop.loop
  // @code
  // loop_->removeChannel(channel_.get());
  // @endcode
  loop_->QueueInLoop(bind(&Connection::ConnectionDestroyed, connection_));
}
