#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>

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
    typedef boost::function<void(const boost::shared_ptr<Connection>&)> WriteCompleteCallback;

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
    void ConnectionEstablished();
    void ConnectionDestroyed();

    EventLoop* loop() {
      return loop_;
    }

    void Send(const void* message, int len);

    void Shutdown();
    bool connected() {
      return kConnected == state_;
    }

    void set_connection_callback(const ConnectionCallback& cb);
    void set_message_callback(const MessageCallback& cb);
    void set_write_complete_callback(const WriteCompleteCallback& cb);
    void set_close_callback(const CloseCallback& cb);

    Buffer input_buffer() {
      return input_buffer_;
    }

    Buffer output_buffer() {
      return output_buffer_;
    }

    const boost::any& context() const {
      return context_;
    }

    void set_context(const boost::any& context) {
      context_ = context; 
    }

  private:
    // connection state
    enum State{kDisconnected, kConnecting, kConnected, kDisconnecting};

    void HandleReadEvent();
    void HandleWriteEvent();
    void HandleCloseEvent();
    void HandleErrorEvent();

    void SendInLoop(const void* message, int len);

    EventLoop *loop_;

    std::string name_;
    InetAddress peeraddr_;
    boost::scoped_ptr<Socket> socket_;
    boost::scoped_ptr<Channel> channel_;

    State state_;

    // buffer.
    Buffer input_buffer_;
    Buffer output_buffer_;

    // context
    boost::any context_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;
};

#endif // NET_CONNECTION_H_

