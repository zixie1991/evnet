#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "connection.h"

class EventLoop;
class InetAddress;
class Acceptor;

class TcpServer {
    public:
        TcpServer(EventLoop* loop, const InetAddress& listen_addr);
        ~TcpServer();

        /**
         * @brief Start the server
         */
        void start();

        void set_connection_callback(const Connection::ConnectionCallback& cb) {
            connection_callback_ = cb;
        }

        void set_message_callback(const Connection::MessageCallback& cb) {
            message_callback_ = cb;
        }

        void set_write_complete_callback(const Connection::WriteCompleteCallback& cb) {
            write_complete_callback_ = cb;
        }

    private:
        void newConnection(int sockfd, const InetAddress& peeraddr);
        void removeConnection(const boost::shared_ptr<Connection>& connection);

        EventLoop* loop_;
        boost::scoped_ptr<Acceptor> acceptor_;
        std::map<std::string, boost::shared_ptr<Connection> > connections_;
        bool started_;
        int next_connection_id_;

        Connection::ConnectionCallback connection_callback_;
        Connection::MessageCallback message_callback_;
        Connection::WriteCompleteCallback write_complete_callback_;
};

#endif // TCPSERVER_H_
