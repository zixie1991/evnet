#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "inetaddress.h"

class EventLoop;
class Socket;
class Channel;

class Connection {
    public:
        typedef boost::function<void(const Connection*)> ConnectionCallback;
        typedef boost::function<void(const Connection*, const char* data, \
                int len)> MessageCallback;
        typedef boost::function<void(const Connection*)> CloseCallback;

        Connection(EventLoop* loop, std::string name, int sockfd, const \
                InetAddress& peeraddr);
        ~Connection();

        const std::string& name() const {
            return name_;
        }

        const InetAddress& peeraddr() const {
            return peeraddr_;
        }

        // called when accept a new connection
        // should be called only once
        void connectionEstablished();

        EventLoop* loop() {
            return loop_;
        }

        void set_connection_callback(const ConnectionCallback& cb);
        void set_message_callback(const MessageCallback& cb);
        void set_close_callback(const CloseCallback& cb);

    private:
        void handleReadEvent();
        void handleWriteEvent();
        void handleCloseEvent();
        void handleErrorEvent();

        EventLoop *loop_;

        std::string name_;
        InetAddress peeraddr_;
        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;

        ConnectionCallback connection_callback_;
        MessageCallback message_callback_;
        CloseCallback close_callback_;
};

#endif // CONNECTION_H_