#include "evnet/http/http_response.h"
#include "evnet/http/http_request.h"
#include "evnet/tcp_client.h"

#include "tunnel.h"

Tunnel::Tunnel(EventLoop* loop, const InetAddress& server_addr, const shared_ptr<TcpConnection>& client_connection):
  client_(new TcpClient(loop, server_addr)),
  client_connection_(client_connection),
  len_(-1)
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
  if (len_ == -1) {
    if (buffer.ReadInt32(len_) < 0) {
      return ;
    }
  }

  string message(len_, ' ');
  if (len_ > 0) {
    if (buffer.Read(const_cast<char*>(message.data()), message.size()) < 0) {
      return ;
    }
  }

  len_ = -1;
  HttpResponse response;
  response.set_version("HTTP/1.1");
  response.set_status_code(200);
  response.add_header("Server", "evnet http_server");
  const shared_ptr<HttpRequest> request = any_cast<const shared_ptr<HttpRequest>>(connection->context(2));
  string conn = request->get_header("Connection");
  if (conn != "") {
    response.add_header("Connection", conn);
  }
  response.set_body(message);
  client_connection_->Send(response.GenResponse());

  bool close = conn == "close" || (request->version() == "HTTP/1.0" && conn != "Keep-Alive");
  if (close) {
    connection->Shutdown();
    client_connection_->Shutdown();
  }
}
