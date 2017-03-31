#ifndef EVNET_TCPCLIENT_H_
#define EVNET_TCPCLIENT_H_

#include "common.h"
#include "tcp_connection.h"

class EventLoop;
class InetAddress;
class TcpConnection;
class Connector;

class TcpClient {
  public:
    TcpClient(EventLoop* loop, const InetAddress server_addr);
    ~TcpClient();

    void Connect();
    void Disconnect();

    void set_connection_callback(const TcpConnection::ConnectionCallback& cb) {
      connection_callback_ = cb;
    }

    void set_message_callback(const TcpConnection::MessageCallback& cb) {
      message_callback_ = cb;
    }

    void set_write_complete_callback(const \
            TcpConnection::WriteCompleteCallback& cb) {
      write_complete_callback_ = cb;
    }

  private:
    void NewConnection(int sockfd, const InetAddress& server_addr);
    void RemoveConnection(const shared_ptr<TcpConnection>& connection);

    EventLoop *loop_;
    unique_ptr<Connector> connector_;

    shared_ptr<TcpConnection> connection_;

    TcpConnection::ConnectionCallback connection_callback_;
    TcpConnection::MessageCallback message_callback_;
    TcpConnection::WriteCompleteCallback write_complete_callback_;
};

#endif

