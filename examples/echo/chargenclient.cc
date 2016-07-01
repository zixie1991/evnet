#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/connector.h"
#include "net/connection.h"
#include "net/buffer.h"
#include "util/log.h"

using std::map;
using std::string;

using boost::scoped_ptr;
using boost::shared_ptr;
using boost::bind;

class ChargenClient {
  public:
    ChargenClient(EventLoop* loop, InetAddress server_addr);
    ~ChargenClient();

    void Start(int num_clients, int data_size);

  private:
    void NewConnection(int sockfd, const InetAddress& peeraddr);
    void OnConnection(const shared_ptr<Connection>& connection);
    void OnMessage(const shared_ptr<Connection>& connection, Buffer& buffer);
    void OnWriteComplete(const shared_ptr<Connection>& connection);
    void RemoveConnection(const shared_ptr<Connection>& connection);

    EventLoop *loop_;
    
    scoped_ptr<Connector> connector_;
    map<string, shared_ptr<Connection> > connections_;
    int next_connection_id_;

    bool started_;
    char *buf;
};

int main(int argc, char *argv[]) {
  if (5 != argc) {
    fprintf(stderr, "Usage: %s <ip> <port> <num_clients> <data_size>\n", argv[0]);
    exit(0);
  }

  set_log_level(Logger::LEVEL_WARN);
  string ip(argv[1]);
  uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
  int num_clients = atoi(argv[3]);
  int data_size = atoi(argv[4]);
  InetAddress server_addr(ip, port);

  EventLoop loop;
  ChargenClient echo_client(&loop, server_addr);

  echo_client.Start(num_clients, data_size);

  return 0;
}

ChargenClient::ChargenClient(EventLoop* loop, InetAddress server_addr):
  loop_(loop),
  connector_(new Connector(server_addr)),
  next_connection_id_(1),
  started_(false)
{
  connector_->set_new_connection_callback(bind(&ChargenClient::NewConnection, this, _1, _2));
}

ChargenClient::~ChargenClient() {
  if (NULL != buf) {
      delete buf;
  }
}

void ChargenClient::Start(int num_clients, int data_size) {
  if (started_) {
    return ;
  }

  started_ = true;
  buf = new char[data_size + 1];
  memset(buf, 'a', data_size);
  buf[data_size] = '\0';
  for (int i = 0; i < num_clients; i++) {
    connector_->Start();
  }

  loop_->Loop();
}

void ChargenClient::NewConnection(int sockfd, const InetAddress& peeraddr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", next_connection_id_);
  ++next_connection_id_;
  string connection_name = buf;

  log_info("Established a new connection %s with %s:%d", connection_name.c_str(), \
          peeraddr.ip().c_str(), peeraddr.port());

  shared_ptr<Connection> connection(new Connection(loop_, connection_name, \
              sockfd, peeraddr));
  connections_[connection_name] = connection;
  connection->set_connection_callback(bind(&ChargenClient::OnConnection, this, _1));
  connection->set_message_callback(bind(&ChargenClient::OnMessage, this, _1, _2));
  connection->set_write_complete_callback(bind(&ChargenClient::OnWriteComplete, this, _1));
  connection->set_close_callback(bind(&ChargenClient::RemoveConnection, this, _1));
  connection->ConnectionEstablished();
}

void ChargenClient::OnConnection(const shared_ptr<Connection>& connection) {
  // send message "hello, world"
  connection->Send(buf, strlen(buf));
}

void ChargenClient::OnWriteComplete(const shared_ptr<Connection>& connection) {
  // send message "hello, world"
  connection->Send(buf, strlen(buf));
}

void ChargenClient::OnMessage(const shared_ptr<Connection>& connection, Buffer& buffer) {
  string message;
  buffer.Read(message);
  connection->Send(message.c_str(), message.size());
}

void ChargenClient::RemoveConnection(const shared_ptr<Connection>& connection) {
  log_info("close callback: remote peer[%s] closed connection", \
          connection->name().c_str());

  log_trace("before connections_.erase(%s): %d connections", connection->name().c_str(), connections_.size());
  size_t n = connections_.erase(connection->name());
  (void)n;
  assert(1 == n);
  log_trace("after connections_.erase(%s): %d connections", connection->name().c_str(), connections_.size());


  // remove channel on next eventloop.loop
  // @code
  // loop_->RemoveChannel(channel_.get());
  // @endcode
}
