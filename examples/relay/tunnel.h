#ifndef TUNNEL_H_
#define TUNNEL_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class EventLoop;
class InetAddress;
class Buffer;
class Connection;
class TcpClient;

class Tunnel {
    public:
        Tunnel(EventLoop* loop, const InetAddress& server_addr, const boost::shared_ptr<Connection>& client_connection);
        ~Tunnel();

        void connect();
        void disconnect();

    private:
        void onServerConnection(const boost::shared_ptr<Connection>& connection);
        void onServerMessage(const boost::shared_ptr<Connection>& connection, \
                Buffer& buffer);

        boost::scoped_ptr<TcpClient> client_;
        boost::shared_ptr<Connection> client_connection_;
};

#endif // TUNNEL_H_
