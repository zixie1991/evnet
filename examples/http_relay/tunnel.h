#ifndef TUNNEL_H_
#define TUNNEL_H_

#include "evnet/common.h"
#include "evnet/tcp_connection.h"

class EventLoop;
class InetAddress;
class Buffer;
class TcpClient;

// 隧道
class Tunnel {
  public:
    Tunnel(EventLoop* loop, const InetAddress& server_addr, const shared_ptr<TcpConnection>& client_connection);
    ~Tunnel();

    void Connect();
    void Disconnect();

  private:
    void OnServerConnection(const shared_ptr<TcpConnection>& connection);
    void OnServerMessage(const shared_ptr<TcpConnection>& connection, \
            Buffer& buffer);

    unique_ptr<TcpClient> client_;
    shared_ptr<TcpConnection> client_connection_;

    int32_t len_;
};

#endif // TUNNEL_H_
