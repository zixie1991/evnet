#include "acceptor.h"
#include "event_loop.h"
#include "tcp_server.h"

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr):
  loop_(loop),
  acceptor_(new Acceptor(loop, listen_addr)),
  started_(false),
  next_connection_id_(1)
{
  acceptor_->set_new_connection_callback_(bind(&TcpServer::NewConnection, this, _1, _2));
}

TcpServer::~TcpServer() {

}

void TcpServer::Start() {
  if (!started_) {
    started_ = true;

    acceptor_->Listen();
  }
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peeraddr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", next_connection_id_);
  ++next_connection_id_;
  string connection_name = buf;

  LOG(INFO) << "Accepted a new connection " << connection_name << " from " << peeraddr.ip() << ":" << peeraddr.port();

  shared_ptr<TcpConnection> connection(new TcpConnection(loop_, connection_name, \
              sockfd, peeraddr));
  connections_[connection_name] = connection;
  connection->set_connection_callback(connection_callback_);
  connection->set_message_callback(message_callback_);
  connection->set_write_complete_callback(write_complete_callback_);
  connection->set_close_callback(bind(&TcpServer::RemoveConnection, this, _1));
  connection->ConnectionEstablished();
}

void TcpServer::RemoveConnection(const shared_ptr<TcpConnection>& connection) {
  LOG(INFO) << "close callback: remote peer[" << connection->name() << "] closed connection";

  size_t n = connections_.erase(connection->name());
  (void)n;
  CHECK(1 == n);

  // remove channel on next eventloop.loop
  // @code
  // loop_->removeChannel(channel_.get());
  // @endcode
  loop_->QueueInLoop(bind(&TcpConnection::ConnectionDestroyed, connection));
}
