#ifndef EVNET_CONNECTOR_H_
#define EVNET_CONNECTOR_H_

#include "common.h"
#include "inet_address.h"
#include "tcp_connection.h"

class Channel;

/**
 * @brief 主动发起连接
 */
class Connector {
  public:
    typedef function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

    Connector(const InetAddress& server_addr);
    ~Connector();

    void Start();
    void Restart();
    void Stop();

    const InetAddress& server_addr() const {
        return server_addr_;
    }

    void set_new_connection_callback(const NewConnectionCallback& cb);

  private:
    void Connect();
    void Connecting();

    InetAddress server_addr_;
    int sockfd_;
    bool connect_;

    NewConnectionCallback new_connection_callback_;
};

#endif
