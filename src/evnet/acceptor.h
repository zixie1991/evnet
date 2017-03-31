#ifndef EVNET_ACCEPTOR_H_
#define EVNET_ACCEPTOR_H_

#include "common.h"
#include "socket.h"
#include "channel.h"

class EventLoop;
class InetAddress;

class Acceptor {
  public:
    typedef function<void(int, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr);
    ~Acceptor();

    void set_new_connection_callback_(NewConnectionCallback cb);

    void Listen();

    bool listenning() {
      return listenning_;
    }

  private:
    void HandleReadEvent();

    EventLoop* loop_;
    Socket socket_;
    Channel channel_;
    bool listenning_;

    // EMFILE errno
    int dummy_fd_;

    NewConnectionCallback new_connection_callback_;
};

#endif
