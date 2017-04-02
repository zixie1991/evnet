#include <signal.h>

#include "evnet/common.h"
#include "evnet/http/http_server.h"
#include "evnet/http/http_request.h"
#include "evnet/event_loop.h"
#include "evnet/tcp_connection.h"

#include "tunnel.h"

class RelayServer {
  public:
    RelayServer(EventLoop* loop, const InetAddress& listen_addr, const InetAddress& remote_addr);
    ~RelayServer();

    void Start();

  private:
    void OnHttpRequest(const shared_ptr<TcpConnection>& connection, const shared_ptr<HttpRequest>& request);

    EventLoop* loop_;
    HttpServer server_;
    InetAddress remote_addr_;

    // tunnel manager
    map<std::string, shared_ptr<Tunnel> > tunnels_;
};

void SigPipe(int signo) {
    // ignore SIGPIPE
    (void)signo;
    return ;
}

int main(int argc, char* argv[]) {
  if (4 != argc) {
      fprintf(stderr, "Usage: %s <listen_port> <server_ip> <server_port>\n", \
              argv[0]);
      exit(0);
  }

  signal(SIGPIPE, SigPipe);
  uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
  InetAddress listen_addr(port);
  string ip(argv[2]);
  port = static_cast<uint16_t>(atoi(argv[3]));
  InetAddress server_addr(ip, port);
  EventLoop loop;
  
  RelayServer server(&loop, listen_addr, server_addr);
  server.Start();
  loop.Run();

  return 0;
}

RelayServer::RelayServer(EventLoop* loop, const InetAddress& listen_addr, const InetAddress& remote_addr):
  loop_(loop),
  server_(loop, listen_addr),
  remote_addr_(remote_addr)
{
  server_.set_http_callback(bind(&RelayServer::OnHttpRequest, this, _1, _2));
}

RelayServer::~RelayServer() {
}

void RelayServer::Start() {
  server_.Start();
}

void RelayServer::OnHttpRequest(const shared_ptr<TcpConnection>& connection, const shared_ptr<HttpRequest>& request) {
  if (connection->context().empty()) {
    shared_ptr<Tunnel> tunnel(new Tunnel(loop_, remote_addr_, connection)); 
    tunnel->Connect();
    tunnels_[connection->name()] = tunnel;
  }

  if (connection->context().empty()) {
    connection->Shutdown();
    return ;
  }

  const shared_ptr<TcpConnection>& remote_connection = any_cast<const shared_ptr<TcpConnection>&>(connection->context());
  remote_connection->set_context(2, request);
  int32_t len = request->body().size();
  len = htobe32(len);
  remote_connection->Send(reinterpret_cast<char*>(&len), sizeof(len));
  string message = request->body();
  remote_connection->Send(message);
}
