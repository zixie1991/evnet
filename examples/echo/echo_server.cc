#include <signal.h>

#include "evnet/tcp_server.h"
#include "evnet/event_loop.h"

class EchoServer {
  public:
    EchoServer(EventLoop* loop, const InetAddress& listen_addr);
    ~EchoServer();

    void Start();

  private:
    void OnConnection(const shared_ptr<TcpConnection>& connection);
    void OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer);

    TcpServer server_;
};

void SigPipe(int signo) {
  // ignore SIGPIPE
  (void)signo;
  return ;
}

int main(int argc, char *argv[]) {
  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(0);
  }

  signal(SIGPIPE, SigPipe);
  uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
  InetAddress listen_addr(port);
  EventLoop loop;

  EchoServer echo_server(&loop, listen_addr);
  echo_server.Start();
  loop.Run();

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
  LOG(INFO) << "echo_server start.";
  server_.Start();
}

void EchoServer::OnConnection(const shared_ptr<TcpConnection>& connection) {
  if (connection->connected()) {
    DLOG(INFO) << "Connection callback: new connection [" << connection->name() << "] from " << connection->peeraddr().ip() << ":" << connection->peeraddr().port() << ".";
  }
}

void EchoServer::OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer) {
  DLOG(INFO) << "Message callback: received " << buffer.ReadableBytes() << " bytes from connection [" << connection->name() << "]";

  string message;
  buffer.Read(message);
  connection->Send(message.c_str(), message.size());
}
