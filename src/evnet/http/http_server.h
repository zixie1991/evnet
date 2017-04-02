#ifndef EVNET_HTTP_HTTP_SERVER_H_
#define EVNET_HTTP_HTTP_SERVER_H_

#include "../common.h"
#include "../tcp_server.h"

class HttpRequest;
class HttpResponse;
class EventLoop;
class InetAddress;

class HttpServer {
  public:
    typedef function<void(const HttpRequest&, HttpResponse*)> HttpCallback;
    typedef function<void(const shared_ptr<TcpConnection>&, const shared_ptr<HttpRequest>&)> AsyncHttpCallback;

    HttpServer(EventLoop* loop, const InetAddress& listen_addr);
    ~HttpServer();

    void Start();

    void set_http_callback(const HttpCallback& cb);
    void set_http_callback(const AsyncHttpCallback& cb);

  private:
    void OnConnection(const shared_ptr<TcpConnection>& connection);
    void OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer);
    void OnRequest(const shared_ptr<TcpConnection>& connection, const shared_ptr<HttpRequest>& request);

    EventLoop* loop_;

    TcpServer server_;
    HttpCallback http_callback_;
    AsyncHttpCallback async_http_callback_;
};

#endif
