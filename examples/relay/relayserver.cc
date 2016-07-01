#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/bind.hpp>

#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/connection.h"
#include "net/buffer.h"
#include "net/tcpserver.h"
#include "util/ptime.h"
#include "util/log.h"

#include "tunnel.h"

using boost::shared_ptr;
using boost::any_cast;
using boost::bind;
using std::string;

class RelayServer {
  public:
    RelayServer(EventLoop* loop, const InetAddress& listen_addr, \
            const InetAddress& server_addr);
    ~RelayServer();

    void Start();

  private:
    void OnClientConnection(const boost::shared_ptr<Connection>& connection);
    void OnClientMessage(const boost::shared_ptr<Connection>& connection, \
            Buffer& buffer);
    void OnClientWriteComplete(const boost::shared_ptr<Connection>& connection);
    // transmit callback
    void Transmit(double seconds);

    EventLoop* loop_;
    TcpServer server_;
    InetAddress remote_server_addr_;

    // tunnel manager
    std::map<std::string, boost::shared_ptr<Tunnel> > tunnels_;

    // transmit count
    long transmit_count_;
};

void sig_pipe(int signo) {
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

    set_log_level(Logger::LEVEL_INFO);
    signal(SIGPIPE, sig_pipe);
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress listen_addr(port);
    string ip(argv[2]);
    port = static_cast<uint16_t>(atoi(argv[3]));
    InetAddress server_addr(ip, port);
    EventLoop loop;
    
    RelayServer server(&loop, listen_addr, server_addr);
    server.Start();
    loop.Loop();

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
  loop_->RunRepeat(time, seconds, bind(&RelayServer::Transmit, this, seconds));

  server_.Start();
}

void RelayServer::OnClientConnection(const boost::shared_ptr<Connection>& \
        connection) {
  if (connection->connected()) {
    shared_ptr<Tunnel> tunnel(new Tunnel(loop_, remote_server_addr_, connection)); 
    tunnel->Connect();
    tunnels_[connection->name()] = tunnel;
  } else {
    log_info("connection %s disconnected.", connection->name().c_str());
    // client ---C--- relayserver ---S--- server
    // when client disconnect connection C, the relayserver would disconnect
    // the connection S
    tunnels_[connection->name()]->Disconnect();
    tunnels_.erase(connection->name());
  }
}

void RelayServer::OnClientMessage(const boost::shared_ptr<Connection>& \
        connection, Buffer& buffer) {
  string message;
  buffer.Read(message);

  if (!connection->context().empty()) {
    const shared_ptr<Connection>& server_connection = \
            any_cast<const shared_ptr<Connection>&>(connection->context());
    server_connection->Send(message.c_str(), message.size());
  }
}

void RelayServer::OnClientWriteComplete(const boost::shared_ptr<Connection>& \
        connection) {
  (void)connection;
  transmit_count_++;
}

void RelayServer::Transmit(double seconds) {
  log_info("relay server transmit: %ld transform/seconds", \
          static_cast<long>(transmit_count_ / seconds));
  transmit_count_ = 0;
}
