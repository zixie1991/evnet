#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/connection.h"
#include "net/tcpserver.h"
#include "net/buffer.h"
#include "util/log.h"

using boost::shared_ptr;
using boost::bind;

class EchoServer {
  public:
    EchoServer(EventLoop* loop, const InetAddress& listen_addr);
    ~EchoServer();

    void Start();

  private:
    void OnConnection(const shared_ptr<Connection>& connection);
    void OnMessage(const shared_ptr<Connection>& connection, Buffer& buffer);

    TcpServer server_;
};

void sig_pipe(int signo) {
  // ignore SIGPIPE
  (void)signo;
  return ;
}

int main(int argc, char *argv[]) {
  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(0);
  }

  set_log_level(Logger::LEVEL_WARN);
  signal(SIGPIPE, sig_pipe);
  uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
  InetAddress listen_addr(port);
  EventLoop loop;

  EchoServer echo_server(&loop, listen_addr);
  echo_server.Start();
  loop.Loop();

  return 0;
}

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listen_addr):
  server_(loop, listen_addr)
{
  server_.set_connection_callback(bind(&EchoServer::OnConnection, this, _1));
  server_.set_message_callback(bind(&EchoServer::OnMessage, this, _1, _2));
}

EchoServer::~EchoServer() {

}

void EchoServer::Start() {
  server_.Start();
}

void EchoServer::OnConnection(const shared_ptr<Connection>& connection) {
  log_info("connection callback: new connection [%s] from %s:%d", \
          connection->name().c_str(), connection->peeraddr().ip().c_str(), \
          connection->peeraddr().port());
}

void EchoServer::OnMessage(const shared_ptr<Connection>& connection, Buffer& buffer) {
  log_info("message callback: received %d bytes from connection [%s]", \
          buffer.ReadableBytes(), connection->name().c_str());

  string message;
  buffer.Read(message);
  connection->Send(message.c_str(), message.size());
}
