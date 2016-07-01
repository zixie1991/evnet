#include "tunnel.h"

#include <string>

#include <boost/bind.hpp>

#include "net/tcpclient.h"
#include "util/log.h"

using std::string;
using boost::bind;

Tunnel::Tunnel(EventLoop* loop, const InetAddress& server_addr, const boost::shared_ptr<Connection>& client_connection):
  client_(new TcpClient(loop, server_addr)),
  client_connection_(client_connection)
{
  client_->set_connection_callback(bind(&Tunnel::OnServerConnection, this, _1));
  client_->set_message_callback(bind(&Tunnel::OnServerMessage, this, _1, _2));
}

Tunnel::~Tunnel() {

}

void Tunnel::Connect() {
  client_->Connect();
}

void Tunnel::Disconnect() {
  client_->Disconnect();
}

void Tunnel::OnServerConnection(const boost::shared_ptr<Connection>& connection) {
  if (connection->connected()) {
    client_connection_->set_context(connection);
    if (client_connection_->input_buffer().ReadableBytes() > 0) {
      string message;
      client_connection_->input_buffer().Read(message);
      connection->Send(message.c_str(), message.size());
    }
  } else {
    log_info("connection %s disconnected.", client_connection_->name().c_str());

    // client ---C--- relayserver ---S--- server
    // when server disconnect connection S, the relayserver would disconnect
    // the connection C
    if (client_connection_) {
        client_connection_->Shutdown();
    }
  }
}

void Tunnel::OnServerMessage(const boost::shared_ptr<Connection>& connection, \
        Buffer& buffer) {
  (void)connection;
  string message;
  buffer.Read(message);
  client_connection_->Send(message.c_str(), message.size());
}
