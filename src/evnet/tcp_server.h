#ifndef EVNET_TCPSERVER_H_
#define EVNET_TCPSERVER_H_

#include "common.h"
#include "tcp_connection.h"

class EventLoop;
class InetAddress;
class Acceptor;
class EventLoopThreadPool;

class TcpServer {
  public:
    TcpServer(EventLoop* loop, const InetAddress& listen_addr, int thread_num=0);
    ~TcpServer();

    /**
      * @brief Start the server
      */
    void Start();

    void set_connection_callback(const TcpConnection::ConnectionCallback& cb) {
      connection_callback_ = cb;
    }

    void set_message_callback(const TcpConnection::MessageCallback& cb) {
      message_callback_ = cb;
    }

    void set_write_complete_callback(const TcpConnection::WriteCompleteCallback& cb) {
      write_complete_callback_ = cb;
    }

  private:
    void NewConnection(int sockfd, const InetAddress& peeraddr);
    void RemoveConnection(const shared_ptr<TcpConnection>& connection);

    //EventLoop* loop_;
    shared_ptr<EventLoopThreadPool> loop_thread_pool_;
    unique_ptr<Acceptor> acceptor_;
    map<string, shared_ptr<TcpConnection> > connections_;
    bool started_;
    int next_connection_id_;

    TcpConnection::ConnectionCallback connection_callback_;
    TcpConnection::MessageCallback message_callback_;
    TcpConnection::WriteCompleteCallback write_complete_callback_;
};

#endif
