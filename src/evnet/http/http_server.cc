#include "http_request.h"
#include "http_response.h"
#include "http_server.h"

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listen_addr):
  server_(loop, listen_addr)
{
  server_.set_connection_callback(bind(&HttpServer::OnConnection, this, _1));
  server_.set_message_callback(bind(&HttpServer::OnMessage, this, _1, _2));
}

HttpServer::~HttpServer() {
}

void HttpServer::Start() {
  server_.Start();
}

void HttpServer::set_http_callback(const HttpCallback& cb) {
  http_callback_ = cb;
}

void HttpServer::set_http_callback(const AsyncHttpCallback& cb) {
  async_http_callback_ = cb;
}

void HttpServer::OnConnection(const shared_ptr<TcpConnection>& connection) {
  if (connection->connected()) {
    shared_ptr<HttpRequest> request(new HttpRequest());
    connection->set_context(1, request);
  }
}

void HttpServer::OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer) {
  shared_ptr<HttpRequest> request = any_cast<shared_ptr<HttpRequest>>(connection->context(1));
  string message;
  buffer.Read(message);
  if (!request->Parse(message.c_str(), message.size())) {
    connection->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    connection->Shutdown();
    return ;
  }

  if (request->ok()) {
    OnRequest(connection, request);
    shared_ptr<HttpRequest> new_request(new HttpRequest());
    connection->set_context(1, new_request);
  }
}

void HttpServer::OnRequest(const shared_ptr<TcpConnection>& connection, const shared_ptr<HttpRequest>& request) {
  if (http_callback_) {
    const string& conn = request->get_header("Connection");
    bool close = conn == "close" || (request->version() == "HTTP/1.0" && conn != "Keep-Alive");

    HttpResponse response;
    http_callback_(*(request.get()), &response);
    string message = response.GenResponse();
    connection->Send(message);
    if (close) {
      connection->Shutdown();
    }
  } else if (async_http_callback_) {
    async_http_callback_(connection, request);
  }
}
