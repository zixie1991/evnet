#include <signal.h>

#include "evnet/tcp_server.h"
#include "evnet/event_loop.h"
#include "evnet/timestamp.h"

#include "tunnel.h"

class RelayServer {
  public:
    RelayServer(EventLoop* loop, const InetAddress& listen_addr, \
            const InetAddress& server_addr);
    ~RelayServer();

    void Start();

  private:
    void OnClientConnection(const shared_ptr<TcpConnection>& connection);
    void OnClientMessage(const shared_ptr<TcpConnection>& connection, \
            Buffer& buffer);
    void OnClientWriteComplete(const shared_ptr<TcpConnection>& connection);
    // transmit callback
    void Transmit(double seconds);

    EventLoop* loop_;
    TcpServer server_;
    InetAddress remote_server_addr_;

    // tunnel manager
    map<std::string, shared_ptr<Tunnel> > tunnels_;

    // transmit count
    long transmit_count_;
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

RelayServer::RelayServer(EventLoop* loop, const InetAddress& listen_addr, \
        const InetAddress& server_addr):
  loop_(loop),
  server_(loop, listen_addr),
  remote_server_addr_(server_addr),
  transmit_count_(0)
{
  server_.set_connection_callback(bind(&RelayServer::OnClientConnection, \
              this, _1));
  server_.set_message_callback(bind(&RelayServer::OnClientMessage, this, \
              _1, _2));
  server_.set_write_complete_callback(bind(&RelayServer::OnClientWriteComplete, this, _1));
}

RelayServer::~RelayServer() {

}

void RelayServer::Start() {
  // 定时器
  Timestamp time;
  time.Now();
  double seconds = 2;
  //loop_->RunRepeat(time, seconds, bind(&RelayServer::Transmit, this, seconds));

  server_.Start();
}

void RelayServer::OnClientConnection(const shared_ptr<TcpConnection>& \
        connection) {
  if (connection->connected()) {
    shared_ptr<Tunnel> tunnel(new Tunnel(loop_, remote_server_addr_, connection)); 
    tunnel->Connect();
    tunnels_[connection->name()] = tunnel;
  } else {
    LOG(INFO) << "Connection [" << connection->name() << "] disconnected";
    // client ---C--- relayserver ---S--- server
    // when client disconnect connection C, the relayserver would disconnect
    // the connection S
    tunnels_[connection->name()]->Disconnect();
    tunnels_.erase(connection->name());
  }
}

void RelayServer::OnClientMessage(const shared_ptr<TcpConnection>& \
        connection, Buffer& buffer) {
  string message;
  buffer.Read(message);

  if (!connection->context().empty()) {
    const shared_ptr<TcpConnection>& server_connection = \
            any_cast<const shared_ptr<TcpConnection>&>(connection->context());
    server_connection->Send(message);
  }
}

void RelayServer::OnClientWriteComplete(const shared_ptr<TcpConnection>& \
        connection) {
  (void)connection;
  transmit_count_++;
}

void RelayServer::Transmit(double seconds) {
  LOG(INFO) << "Relay server transmit: %ld transform/seconds" << static_cast<long>(transmit_count_ / seconds);
  transmit_count_ = 0;
}
