#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <boost/function.hpp>

#include "inetaddress.h"

class EventLoop;

/**
 * @brief 主动发起连接
 */
class Connector {
    public:
        typedef boost::function<void (int sockfd)> ConnectionCallback;

        Connector(EventLoop* loop, const InetAddress& server_addr);
        ~Connector();

        void start();
        void restart();
        void stop();

        const InetAddress7 server_addr() const {
            return server_addr_;
        }

        void set_connection_callback(const ConnectionCallback& cb);

    private:
        void connect();
        void connecting(int sockfd);

        void handleWriteEvent();
        void handleErrorEvent();

        EventLoop* loop_;
        InetAddress server_addr_;
        bool connect_;

        ConnectionCallback connection_callback_;
};

#endif // CONNECTOR_H_
