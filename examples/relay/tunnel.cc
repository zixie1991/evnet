#include "evnet/tcp_client.h"

#include "tunnel.h"

Tunnel::Tunnel(EventLoop* loop, const InetAddress& server_addr, const shared_ptr<TcpConnection>& client_connection):
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

void Tunnel::OnServerConnection(const shared_ptr<TcpConnection>& connection) {
  if (connection->connected()) {
    client_connection_->set_context(connection);
    if (client_connection_->input_buffer().ReadableBytes() > 0) {
      string message;
      client_connection_->input_buffer().Read(message);
      connection->Send(message);
    }
  } else {
    LOG(INFO) << "Connection [" << connection->name() << "] disconnected";
    // client ---C--- relayserver ---S--- server
    // when server disconnect connection S, the relayserver would disconnect
    // the connection C
    if (client_connection_) {
        client_connection_->Shutdown();
    }
  }
}

void Tunnel::OnServerMessage(const shared_ptr<TcpConnection>& connection, \
        Buffer& buffer) {
  (void)connection;
  string message;
  buffer.Read(message);
  client_connection_->Send(message);
}
