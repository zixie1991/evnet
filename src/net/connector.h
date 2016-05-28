#ifndef NET_CONNECTOR_H_
#define NET_CONNECTOR_H_

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "inetaddress.h"
#include "connection.h"

class Channel;

/**
 * @brief 主动发起连接
 */
class Connector {
    public:
        typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

        Connector(const InetAddress& server_addr);
        ~Connector();

        void start();
        void restart();
        void stop();

        const InetAddress& server_addr() const {
            return server_addr_;
        }

        void set_new_connection_callback(const NewConnectionCallback& cb);

    private:
        void connect();
        void connecting();

        InetAddress server_addr_;
        int sockfd_;
        bool connect_;

        NewConnectionCallback new_connection_callback_;
};

#endif // NET_CONNECTOR_H_

