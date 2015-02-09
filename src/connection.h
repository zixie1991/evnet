#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "inetaddress.h"
#include "buffer.h"

class EventLoop;
class Socket;
class Channel;

class Connection: public boost::enable_shared_from_this<Connection> {
    public:
        typedef boost::function<void(const boost::shared_ptr<Connection>&)> ConnectionCallback;
        typedef boost::function<void(const boost::shared_ptr<Connection>&, \
                Buffer& buffer)> MessageCallback;
        typedef boost::function<void(const boost::shared_ptr<Connection>&)> CloseCallback;

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

        void connectionStreamed();

        EventLoop* loop() {
            return loop_;
        }

        void send(const void* message, int len);

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

        // buffer.
        Buffer input_buffer_;
        Buffer output_buffer_;

        ConnectionCallback connection_callback_;
        MessageCallback message_callback_;
        CloseCallback close_callback_;
};

#endif // CONNECTION_H_
