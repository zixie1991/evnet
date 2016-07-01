#ifndef TUNNEL_H_
#define TUNNEL_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class EventLoop;
class InetAddress;
class Buffer;
class Connection;
class TcpClient;

// 隧道
class Tunnel {
  public:
    Tunnel(EventLoop* loop, const InetAddress& server_addr, const boost::shared_ptr<Connection>& client_connection);
    ~Tunnel();

    void Connect();
    void Disconnect();

  private:
    void OnServerConnection(const boost::shared_ptr<Connection>& connection);
    void OnServerMessage(const boost::shared_ptr<Connection>& connection, \
            Buffer& buffer);

    boost::scoped_ptr<TcpClient> client_;
    boost::shared_ptr<Connection> client_connection_;
};

#endif // TUNNEL_H_
