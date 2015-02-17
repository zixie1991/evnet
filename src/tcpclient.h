#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "connection.h"

class EventLoop;
class InetAddress;
class Connection;
class Connector;

class TcpClient {
    public:
        TcpClient(EventLoop* loop, const InetAddress server_addr);
        ~TcpClient();

        void connect();
        void disconnect();

        void set_connection_callback(const Connection::ConnectionCallback& cb) {
            connection_callback_ = cb;
        }

        void set_message_callback(const Connection::MessageCallback& cb) {
            message_callback_ = cb;
        }

        void set_write_complete_callback(const \
                Connection::WriteCompleteCallback& cb) {
            write_complete_callback_ = cb;
        }

    private:
        void newConnection(int sockfd, const InetAddress& server_addr);
        void removeConnection(const boost::shared_ptr<Connection>& connection);

        EventLoop *loop_;
        boost::scoped_ptr<Connector> connector_;

        boost::shared_ptr<Connection> connection_;

        Connection::ConnectionCallback connection_callback_;
        Connection::MessageCallback message_callback_;
        Connection::WriteCompleteCallback write_complete_callback_;
};

#endif // TCPCLIENT_H_
