#ifndef EVNET_CONNECTION_H_
#define EVNET_CONNECTION_H_

#include "common.h"
#include "inet_address.h"
#include "buffer.h"

class EventLoop;
class Socket;
class Channel;

const int kContextCount = 16;

class TcpConnection: public enable_shared_from_this<TcpConnection> {
  public:
    typedef function<void(const shared_ptr<TcpConnection>&)> ConnectionCallback;
    typedef function<void(const shared_ptr<TcpConnection>&, Buffer& buffer)> MessageCallback;
    typedef function<void(const shared_ptr<TcpConnection>&)> CloseCallback;
    typedef function<void(const shared_ptr<TcpConnection>&)> WriteCompleteCallback;

    TcpConnection(EventLoop* loop, string name, int sockfd, const InetAddress& peeraddr);
    ~TcpConnection();

    const string& name() const {
      return name_;
    }

    const InetAddress& peeraddr() const {
      return peeraddr_;
    }

    /**
     * @brief 关闭(true)/开启Nagle算法，允许小封包的发送
     *
     * @param on
     */
    void SetTcpNoDelay(bool on);

    /**
     * @brief 保持存活
     *
     * @param on
     */
    void SetKeepAlive(bool on);

    // called when accept a new connection
    // should be called only once
    void ConnectionEstablished();
    void ConnectionDestroyed();

    EventLoop* loop() {
      return loop_;
    }

    void Send(const void* message, int len);
    void Send(const string& message);

    void Shutdown();
    bool connected() {
      return kConnected == state_;
    }

    const char* StateToString() const;

    void set_connection_callback(const ConnectionCallback& cb);
    void set_message_callback(const MessageCallback& cb);
    void set_write_complete_callback(const WriteCompleteCallback& cb);
    void set_close_callback(const CloseCallback& cb);

    Buffer& input_buffer() {
      return input_buffer_;
    }

    Buffer& output_buffer() {
      return output_buffer_;
    }

    const any& context() const {
      return context_[0];
    }

    void set_context(const any& context) {
      context_[0] = context; 
    }

    const any& context(int idx) const {
      CHECK(idx >= 0 && idx < kContextCount);
      return context_[idx];
    }

    void set_context(int idx, const any& context) {
      CHECK(idx >= 0 && idx < kContextCount);
      context_[idx] = context;
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

    string name_;
    InetAddress peeraddr_;
    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;

    State state_;

    // buffer.
    Buffer input_buffer_;
    Buffer output_buffer_;

    // context
    any context_[kContextCount];

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;
};

#endif
